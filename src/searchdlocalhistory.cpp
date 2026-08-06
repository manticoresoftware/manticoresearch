// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Interactive editing and durable history for configless local mode.

#include "searchdlocalinternal.h"

#include "fileutils.h"

#include <cerrno>
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#if !_WIN32
#include <fcntl.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#if HAVE_DLOPEN
#include <dlfcn.h>
#endif
#endif

namespace localmode
{
#if !_WIN32
constexpr const char * LOCAL_HISTORY = ".history";
constexpr const char * LOCAL_HISTORY_LOCK = ".history.lock";
CSphString g_sLocalDataDir;

CSphString LocalPath ( const char * szName )
{
	CSphString sPath;
	sPath.SetSprintf ( "%s/%s", g_sLocalDataDir.cstr(), szName );
	return sPath;
}

bool ErrnoError ( CSphString & sError, const char * szAction, const char * szPath, int iError=errno, const char * szSuffix="" )
{
	sError.SetSprintf ( "%s '%s'%s: %s", szAction, szPath, szSuffix, strerror(iError) );
	return false;
}

int LocalOpenFlags ( int iFlags, bool bNoFollow=false, bool bDirectory=false )
{
#ifdef O_CLOEXEC
	iFlags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
	if ( bNoFollow ) iFlags |= O_NOFOLLOW;
#endif
#ifdef O_DIRECTORY
	if ( bDirectory ) iFlags |= O_DIRECTORY;
#endif
	return iFlags;
}

volatile sig_atomic_t g_bLineEditorCanJump = 0;
sigjmp_buf g_tLineEditorJump;
using LineEditorSignalCleanup_fn = void (*) ();
using LineEditorInitialize_fn = int (*) ();
using CompletionFunction_fn = char ** (*) ( const char *, int, int );
using CompletionEntry_fn = char * (*) ( const char *, int );
using CompletionMatches_fn = char ** (*) ( const char *, CompletionEntry_fn );
LineEditorSignalCleanup_fn g_fnFreeLineState = nullptr;
LineEditorSignalCleanup_fn g_fnCleanupAfterSignal = nullptr;
LineEditorInitialize_fn g_fnInitialize = nullptr;
int * g_pLineEnd = nullptr;
int * g_pLinePoint = nullptr;
char ** g_ppLineBuffer = nullptr;
CompletionFunction_fn * g_pAttemptedCompletionFunction = nullptr;
int * g_pAttemptedCompletionOver = nullptr;
int * g_pCompletionAppendCharacter = nullptr;
char ** g_ppCompleterWordBreakCharacters = nullptr;
const char ** g_ppBasicWordBreakCharacters = nullptr;

class LocalLineEditor_c;
LocalLineEditor_c * g_pActiveLineEditor = nullptr;

void HandleLineEditorSignal ( int iSignal )
{
	static constexpr char sSigint[] = "^C\n";
	static constexpr char sSigquit[] = "^\\\n";
	if ( iSignal==SIGINT )
		write ( STDOUT_FILENO, sSigint, sizeof(sSigint)-1 );
	else
		write ( STDOUT_FILENO, sSigquit, sizeof(sSigquit)-1 );
	if ( g_bLineEditorCanJump )
	{
		g_bLineEditorCanJump = 0;
		siglongjmp ( g_tLineEditorJump, 1 );
	}
}

struct SavedLineEditorSignals_t
{
	static constexpr int SIGNAL_COUNT = 7;
	int m_dSignals[SIGNAL_COUNT] { SIGINT, SIGTSTP, SIGQUIT, SIGHUP, SIGTERM, SIGCONT, SIGWINCH };
	struct sigaction m_dActions[SIGNAL_COUNT] {};
	bool m_dValid[SIGNAL_COUNT] {};

	void Save()
	{
		for ( int i=0; i<SIGNAL_COUNT; ++i )
			m_dValid[i] = sigaction ( m_dSignals[i], nullptr, &m_dActions[i] )==0;
	}

	void Restore() const
	{
		sigset_t tSignals;
		sigset_t tPrevious;
		sigemptyset ( &tSignals );
		for ( int i=0; i<SIGNAL_COUNT; ++i )
			sigaddset ( &tSignals, m_dSignals[i] );
		bool bBlocked = sigprocmask ( SIG_BLOCK, &tSignals, &tPrevious )==0;
		for ( int i=0; i<SIGNAL_COUNT; ++i )
			if ( m_dValid[i] )
				sigaction ( m_dSignals[i], &m_dActions[i], nullptr );
		if ( bBlocked )
			sigprocmask ( SIG_SETMASK, &tPrevious, nullptr );
	}
};

class LocalLineEditor_c final : public LineEditor_i
{
	using Readline_fn = char * (*) ( const char * );
	using AddHistory_fn = int (*) ( const char * );
	using ClearHistory_fn = void (*) ();
	using StifleHistory_fn = void (*) ( int );
	static constexpr size_t HISTORY_LIMIT = 1000;
	static constexpr off_t MAX_HISTORY_FILE_SIZE = 1024 * 1024;

	class HistoryLock_c
	{
		int m_iFile = -1;

	public:
		~HistoryLock_c() { SafeClose ( m_iFile ); }

		bool Lock ( CSphString & sError )
		{
			CSphString sPath = LocalPath ( LOCAL_HISTORY_LOCK );
			int iFlags = LocalOpenFlags ( O_RDWR | O_CREAT, true );
			m_iFile = open ( sPath.cstr(), iFlags, S_IRUSR | S_IWUSR );
			if ( m_iFile<0 )
				return ErrnoError ( sError, "failed to open local history lock", sPath.cstr() );

			struct stat tStat {};
			if ( fstat ( m_iFile, &tStat )<0 )
				return ErrnoError ( sError, "failed to inspect local history lock", sPath.cstr() );
			if ( !S_ISREG(tStat.st_mode) )
			{
				sError.SetSprintf ( "local history lock '%s' is not a regular file", sPath.cstr() );
				return false;
			}
			if ( fchmod ( m_iFile, S_IRUSR | S_IWUSR )<0 )
				return ErrnoError ( sError, "failed to secure local history lock", sPath.cstr() );

			struct flock tLock {};
			tLock.l_type = F_WRLCK;
			tLock.l_whence = SEEK_SET;
			while ( fcntl ( m_iFile, F_SETLKW, &tLock )<0 )
			{
				if ( errno==EINTR )
					continue;
				return ErrnoError ( sError, "failed to lock local history", sPath.cstr() );
			}
			return true;
		}
	};

	std::unique_ptr<CSphDynamicLibrary> m_pLibrary;
	Readline_fn m_fnReadline = nullptr;
	AddHistory_fn m_fnAddHistory = nullptr;
	ClearHistory_fn m_fnClearHistory = nullptr;
	StifleHistory_fn m_fnStifleHistory = nullptr;
	CompletionMatches_fn m_fnCompletionMatches = nullptr;
	CompletionProvider_fn m_fnCompletionProvider;
	std::vector<std::string> m_dCompletionWords;
	std::vector<std::string> m_dRecentWords;
	std::vector<std::string> m_dCompletionMatches;
	size_t m_iCompletionMatch = 0;
	bool m_bPersistenceEnabled = true;
	std::vector<std::string> m_dHistory;
	size_t m_iUnpersistedHistory = 0;
	CSphString m_sWarning;

	static char * CompletionGenerator ( const char *, int iState )
	{
		if ( !g_pActiveLineEditor )
			return nullptr;
		if ( !iState )
			g_pActiveLineEditor->m_iCompletionMatch = 0;
		if ( g_pActiveLineEditor->m_iCompletionMatch>=g_pActiveLineEditor->m_dCompletionMatches.size() )
			return nullptr;
		return strdup ( g_pActiveLineEditor->m_dCompletionMatches[g_pActiveLineEditor->m_iCompletionMatch++].c_str() );
	}

	static char ** Complete ( const char * szText, int, int )
	{
		if ( !g_pActiveLineEditor || !g_pActiveLineEditor->m_fnCompletionMatches )
			return nullptr;
		if ( g_pAttemptedCompletionOver )
			*g_pAttemptedCompletionOver = 1;
		LocalLineEditor_c & tEditor = *g_pActiveLineEditor;
		tEditor.m_dCompletionMatches.clear();
		size_t iPrefix = strlen ( szText );
		auto AddMatches = [&] ( const std::vector<std::string> & dWords )
		{
			for ( const std::string & sWord : dWords )
			{
				if ( sWord.size()<iPrefix || strncasecmp ( sWord.c_str(), szText, iPrefix )!=0 )
					continue;
				bool bDuplicate = std::any_of ( tEditor.m_dCompletionMatches.begin(), tEditor.m_dCompletionMatches.end(), [&] ( const std::string & sMatch ) { return strcasecmp ( sMatch.c_str(), sWord.c_str() )==0; } );
				if ( !bDuplicate )
					tEditor.m_dCompletionMatches.push_back ( sWord );
			}
		};
		AddMatches ( tEditor.m_dRecentWords );
		AddMatches ( tEditor.m_dCompletionWords );
		return tEditor.m_dCompletionMatches.empty() ? nullptr : tEditor.m_fnCompletionMatches ( szText, CompletionGenerator );
	}

	void RememberSessionWords ( const std::string & sLine )
	{
		char cQuote = 0;
		for ( size_t i=0; i<sLine.size(); )
		{
			if ( cQuote )
			{
				if ( sLine[i]=='\\' && i+1<sLine.size() )
				{
					i += 2;
					continue;
				}
				if ( sLine[i]==cQuote )
				{
					if ( i+1<sLine.size() && sLine[i+1]==cQuote )
					{
						i += 2;
						continue;
					}
					cQuote = 0;
				}
				++i;
				continue;
			}
			if ( sLine[i]=='\'' || sLine[i]=='"' )
			{
				cQuote = sLine[i++];
				continue;
			}
			if ( !isalpha ( (unsigned char)sLine[i] ) && sLine[i]!='_' )
			{
				++i;
				continue;
			}
			size_t iEnd = i+1;
			while ( iEnd<sLine.size() && ( isalnum ( (unsigned char)sLine[iEnd] ) || sLine[iEnd]=='_' || sLine[iEnd]=='$' ) )
				++iEnd;
			std::string sWord = sLine.substr ( i, iEnd-i );
			auto iExisting = std::find_if ( m_dRecentWords.begin(), m_dRecentWords.end(), [&] ( const std::string & sExisting ) { return strcasecmp ( sExisting.c_str(), sWord.c_str() )==0; } );
			if ( iExisting!=m_dRecentWords.end() )
				m_dRecentWords.erase ( iExisting );
			m_dRecentWords.insert ( m_dRecentWords.begin(), std::move(sWord) );
			if ( m_dRecentWords.size()>100 )
				m_dRecentWords.resize ( 100 );
			i = iEnd;
		}
	}

	void InstallCompletion()
	{
		if ( !g_pAttemptedCompletionFunction || !m_fnCompletionMatches )
			return;
		g_pActiveLineEditor = this;
		*g_pAttemptedCompletionFunction = Complete;
		if ( g_pCompletionAppendCharacter )
			*g_pCompletionAppendCharacter = ' ';
		if ( g_ppCompleterWordBreakCharacters )
			*g_ppCompleterWordBreakCharacters = const_cast<char *> ( " 	\n,();=<>+-*/%" );
		if ( g_ppBasicWordBreakCharacters )
			*g_ppBasicWordBreakCharacters = " 	\n,();=<>+-*/%";
	}

	static void TrimHistory ( std::vector<std::string> & dHistory )
	{
		if ( dHistory.size()>HISTORY_LIMIT )
			dHistory.erase ( dHistory.begin(), dHistory.begin() + ( dHistory.size()-HISTORY_LIMIT ) );
	}

	void ReplayHistory()
	{
		m_fnClearHistory();
		for ( const std::string & sCommand : m_dHistory )
			m_fnAddHistory ( sCommand.c_str() );
		m_fnStifleHistory ( HISTORY_LIMIT );
	}

	void RememberUnpersistedHistory ( const std::string & sLine )
	{
		m_dHistory.push_back ( sLine );
		TrimHistory ( m_dHistory );
		m_iUnpersistedHistory = Min ( m_iUnpersistedHistory+1, HISTORY_LIMIT );
		m_fnAddHistory ( sLine.c_str() );
		m_fnStifleHistory ( HISTORY_LIMIT );
	}

	bool LoadLibrary()
	{
#if !HAVE_DLOPEN
		return false;
#else
		static constexpr const char * dLibraries[] =
		{
#ifdef LOCAL_LIBEDIT_LIBRARY
			LOCAL_LIBEDIT_LIBRARY,
#endif
#if __APPLE__
			"libedit.3.dylib", "libedit.dylib",
#else
			"libedit.so", "libedit.so.8", "libedit.so.7", "libedit.so.6", "libedit.so.5",
			"libedit.so.4", "libedit.so.3", "libedit.so.2", "libedit.so.1", "libedit.so.0",
#endif
		};

		for ( const char * szLibrary : dLibraries )
		{
			if ( !szLibrary || !*szLibrary )
				continue;
			if ( !m_pLibrary )
				m_pLibrary = std::make_unique<CSphDynamicLibrary> ( szLibrary, false );
			else if ( !m_pLibrary->GetLib() )
				m_pLibrary->CSphDynamicLibraryAlternative ( szLibrary, false );
			if ( m_pLibrary->GetLib() )
				break;
		}
		if ( !m_pLibrary || !m_pLibrary->GetLib() )
			return false;

		const char * dNames[] = { "readline", "add_history", "clear_history", "stifle_history", "rl_free_line_state", "rl_cleanup_after_signal", "rl_initialize", "rl_end", "rl_point", "rl_line_buffer" };
		void ** dFunctions[] =
		{
			(void**)&m_fnReadline, (void**)&m_fnAddHistory, (void**)&m_fnClearHistory, (void**)&m_fnStifleHistory,
			(void**)&g_fnFreeLineState, (void**)&g_fnCleanupAfterSignal, (void**)&g_fnInitialize,
			(void**)&g_pLineEnd, (void**)&g_pLinePoint, (void**)&g_ppLineBuffer
		};
		if ( !m_pLibrary->LoadSymbols ( dNames, dFunctions, sizeof(dFunctions)/sizeof(dFunctions[0]) ) )
			return false;
		m_fnCompletionMatches = (CompletionMatches_fn)dlsym ( m_pLibrary->GetLib(), "rl_completion_matches" );
		g_pAttemptedCompletionFunction = (CompletionFunction_fn*)dlsym ( m_pLibrary->GetLib(), "rl_attempted_completion_function" );
		g_pAttemptedCompletionOver = (int*)dlsym ( m_pLibrary->GetLib(), "rl_attempted_completion_over" );
		g_pCompletionAppendCharacter = (int*)dlsym ( m_pLibrary->GetLib(), "rl_completion_append_character" );
		g_ppCompleterWordBreakCharacters = (char**)dlsym ( m_pLibrary->GetLib(), "rl_completer_word_break_characters" );
		g_ppBasicWordBreakCharacters = (const char**)dlsym ( m_pLibrary->GetLib(), "rl_basic_word_break_characters" );
		return true;
#endif
	}

	static bool WriteAll ( int iFile, const char * pData, size_t iLength )
	{
		while ( iLength )
		{
			ssize_t iWritten = write ( iFile, pData, iLength );
			if ( iWritten>0 )
			{
				pData += iWritten;
				iLength -= iWritten;
				continue;
			}
			if ( iWritten<0 && errno==EINTR )
				continue;
			if ( !iWritten )
				errno = EIO;
			return false;
		}
		return true;
	}

	bool LoadHistory ( std::vector<std::string> & dHistory, CSphString & sError )
	{
		dHistory.clear();
		CSphString sPath = LocalPath ( LOCAL_HISTORY );
		struct stat tStat {};
		if ( lstat ( sPath.cstr(), &tStat )<0 )
		{
			if ( errno==ENOENT )
				return true;
			return ErrnoError ( sError, "failed to inspect local history", sPath.cstr() );
		}
		if ( !S_ISREG(tStat.st_mode) )
		{
			sError.SetSprintf ( "local history '%s' is not a regular file", sPath.cstr() );
			return false;
		}

		int iFlags = LocalOpenFlags ( O_RDONLY, true );
		int iFile = open ( sPath.cstr(), iFlags );
		if ( iFile<0 )
			return ErrnoError ( sError, "failed to open local history", sPath.cstr() );
		AT_SCOPE_EXIT ( [&] { if ( iFile>=0 ) close ( iFile ); } );

		struct stat tOpenedStat {};
		if ( fstat ( iFile, &tOpenedStat )<0 )
			return ErrnoError ( sError, "failed to inspect opened local history", sPath.cstr() );
		if ( !S_ISREG(tOpenedStat.st_mode) || tOpenedStat.st_dev!=tStat.st_dev || tOpenedStat.st_ino!=tStat.st_ino )
		{
			sError.SetSprintf ( "local history '%s' changed while opening it", sPath.cstr() );
			return false;
		}
		if ( tOpenedStat.st_size>MAX_HISTORY_FILE_SIZE )
		{
			sError.SetSprintf ( "failed to read local history '%s': invalid or unsupported history format", sPath.cstr() );
			return false;
		}
		if ( fchmod ( iFile, S_IRUSR | S_IWUSR )<0 )
			return ErrnoError ( sError, "failed to secure local history", sPath.cstr() );

		FILE * pFile = fdopen ( iFile, "r" );
		if ( !pFile )
			return ErrnoError ( sError, "failed to open local history stream", sPath.cstr() );
		iFile = -1; // owned by pFile

		char * pLine = nullptr;
		size_t iCapacity = 0;
		ssize_t iLength = 0;
		bool bValid = true;
		bool bFirstContent = true;
		while ( ( iLength = getline ( &pLine, &iCapacity, pFile ) )>=0 )
		{
			std::string sLine ( pLine, (size_t)iLength );
			if ( sLine.find ( '\0' )!=std::string::npos )
			{
				bValid = false;
				break;
			}
			while ( !sLine.empty() && ( sLine.back()=='\n' || sLine.back()=='\r' ) )
				sLine.pop_back();

			constexpr const char * LIBEDIT_HISTORY_MARKER = "_HiStOrY_V2_";
			size_t iMarker = 0;
			while ( true )
			{
				while ( iMarker<sLine.size() && isspace ( (unsigned char)sLine[iMarker] ) )
					++iMarker;
				if ( iMarker+3>sLine.size() || memcmp ( sLine.data()+iMarker, "\xEF\xBB\xBF", 3 )!=0 )
					break;
				iMarker += 3;
			}
			if ( iMarker==sLine.size() )
				continue;
			if ( bFirstContent )
			{
				bool bUtf16 = iMarker+2<=sLine.size() &&
					( memcmp ( sLine.data()+iMarker, "\xFE\xFF", 2 )==0 || memcmp ( sLine.data()+iMarker, "\xFF\xFE", 2 )==0 );
				if ( bUtf16 || sLine.compare ( iMarker, strlen(LIBEDIT_HISTORY_MARKER), LIBEDIT_HISTORY_MARKER )==0 )
				{
					bValid = false;
					break;
				}
				bFirstContent = false;
			}
			dHistory.push_back ( std::move(sLine) );
		}
		if ( ferror(pFile) )
		{
			ErrnoError ( sError, "failed to read local history", sPath.cstr(), errno ? errno : EIO );
			bValid = false;
		}
		free ( pLine );
		if ( fclose ( pFile )<0 && bValid )
		{
			ErrnoError ( sError, "failed to close local history", sPath.cstr() );
			bValid = false;
		}
		if ( !bValid )
		{
			dHistory.clear();
			if ( sError.IsEmpty() )
				sError.SetSprintf ( "failed to read local history '%s': invalid or unsupported history format", sPath.cstr() );
			return false;
		}

		TrimHistory ( dHistory );
		return true;
	}

	bool SaveHistory ( const std::vector<std::string> & dHistory, CSphString & sError, bool & bReplaced )
	{
		bReplaced = false;
		CSphString sPath = LocalPath ( LOCAL_HISTORY );
		CSphString sTemporary;
		sTemporary.SetSprintf ( "%s.tmp.XXXXXX", sPath.cstr() );
		std::vector<char> dTemporary ( sTemporary.cstr(), sTemporary.cstr()+sTemporary.Length()+1 );
		int iFile = mkstemp ( dTemporary.data() );
		if ( iFile<0 )
			return ErrnoError ( sError, "failed to create local history", sPath.cstr() );
		AT_SCOPE_EXIT ( [&] { if ( iFile>=0 ) close ( iFile ); if ( !bReplaced ) unlink ( dTemporary.data() ); } );

		if ( fchmod ( iFile, S_IRUSR | S_IWUSR )<0 )
			return ErrnoError ( sError, "failed to secure temporary local history", dTemporary.data() );
		for ( const std::string & sCommand : dHistory )
			if ( !WriteAll ( iFile, sCommand.data(), sCommand.size() ) || !WriteAll ( iFile, "\n", 1 ) )
				return ErrnoError ( sError, "failed to write local history", dTemporary.data() );
		if ( fsync ( iFile )<0 )
			return ErrnoError ( sError, "failed to sync temporary local history", dTemporary.data() );
		if ( close ( iFile )<0 )
		{
			int iError = errno;
			iFile = -1;
			return ErrnoError ( sError, "failed to close written local history", dTemporary.data(), iError );
		}
		iFile = -1;
		if ( rename ( dTemporary.data(), sPath.cstr() )<0 )
			return ErrnoError ( sError, "failed to replace local history", sPath.cstr() );
		bReplaced = true;

		int iDirFlags = LocalOpenFlags ( O_RDONLY, false, true );
		int iDirectory = open ( g_sLocalDataDir.cstr(), iDirFlags );
		if ( iDirectory<0 )
			return ErrnoError ( sError, "failed to open local data directory", g_sLocalDataDir.cstr(), errno, " after saving history" );
		if ( fsync ( iDirectory )<0 )
		{
			int iError = errno;
			close ( iDirectory );
			return ErrnoError ( sError, "failed to sync local data directory", g_sLocalDataDir.cstr(), iError, " after saving history" );
		}
		if ( close ( iDirectory )<0 )
			return ErrnoError ( sError, "failed to close local data directory", g_sLocalDataDir.cstr(), errno, " after saving history" );
		return true;
	}

public:
	explicit LocalLineEditor_c ( CompletionProvider_fn fnCompletionProvider )
		: m_fnCompletionProvider ( std::move(fnCompletionProvider) )
	{
		if ( !LoadLibrary() )
			return;
		m_fnStifleHistory ( HISTORY_LIMIT );
		HistoryLock_c tLock;
		if ( !tLock.Lock ( m_sWarning ) )
			return;
		if ( !LoadHistory ( m_dHistory, m_sWarning ) )
			m_bPersistenceEnabled = false;
		ReplayHistory();
		InstallCompletion();
	}

	~LocalLineEditor_c() override
	{
		if ( g_pActiveLineEditor==this )
			g_pActiveLineEditor = nullptr;
		if ( g_pAttemptedCompletionFunction )
			*g_pAttemptedCompletionFunction = nullptr;
	}

	bool IsActive() const { return m_fnReadline && m_fnAddHistory && m_fnClearHistory && m_fnStifleHistory && g_fnFreeLineState && g_fnCleanupAfterSignal && g_fnInitialize && g_pLineEnd && g_pLinePoint && g_ppLineBuffer; }
	const CSphString & GetWarning() const { return m_sWarning; }

	bool Read ( std::string & sLine ) override
	{
		while ( true )
		{
			// Metadata requests own sockets and C++ scope guards; never run them from
			// a LibEdit callback while signal recovery may siglongjmp.
			if ( m_dCompletionWords.empty() && m_fnCompletionProvider && m_fnCompletionMatches && g_pAttemptedCompletionFunction )
				m_dCompletionWords = m_fnCompletionProvider ( "" );
			InstallCompletion();
			SavedLineEditorSignals_t tSavedSignals;
			tSavedSignals.Save();
			struct sigaction tHandler {};
			sigemptyset ( &tHandler.sa_mask );
			sigaddset ( &tHandler.sa_mask, SIGINT );
			sigaddset ( &tHandler.sa_mask, SIGQUIT );
			tHandler.sa_handler = HandleLineEditorSignal;
			sigaction ( SIGINT, &tHandler, nullptr );
			sigaction ( SIGQUIT, &tHandler, nullptr );
			if ( sigsetjmp ( g_tLineEditorJump, 1 ) )
			{
				g_bLineEditorCanJump = 0;
				tSavedSignals.Restore();
				if ( g_ppLineBuffer && *g_ppLineBuffer && g_pLineEnd && g_pLinePoint )
				{
					**g_ppLineBuffer = '\0';
					*g_pLineEnd = 0;
					*g_pLinePoint = 0;
				}
				g_fnFreeLineState();
				g_fnCleanupAfterSignal();
				g_fnInitialize();
				ReplayHistory();
				InstallCompletion();
				continue;
			}
			g_bLineEditorCanJump = 1;
			char * szLine = m_fnReadline ( "manticore> " );
			g_bLineEditorCanJump = 0;
			tSavedSignals.Restore();
			if ( !szLine )
			{
				fputc ( '\n', stdout );
				return false;
			}
			sLine = szLine;
			free ( szLine );
			return true;
		}
	}

	bool AddHistory ( const std::string & sLine, CSphString & sError ) override
	{
		if ( !m_bPersistenceEnabled )
		{
			RememberUnpersistedHistory ( sLine );
			return true;
		}

		HistoryLock_c tLock;
		if ( !tLock.Lock ( sError ) )
		{
			RememberUnpersistedHistory ( sLine );
			return false;
		}
		std::vector<std::string> dHistory;
		if ( !LoadHistory ( dHistory, sError ) )
		{
			m_bPersistenceEnabled = false;
			RememberUnpersistedHistory ( sLine );
			return false;
		}
		size_t iPendingStart = m_dHistory.size() - Min ( m_iUnpersistedHistory, m_dHistory.size() );
		dHistory.insert ( dHistory.end(), m_dHistory.begin()+iPendingStart, m_dHistory.end() );
		dHistory.push_back ( sLine );
		TrimHistory ( dHistory );
		m_dHistory = std::move ( dHistory );
		ReplayHistory();
		m_iUnpersistedHistory = Min ( m_iUnpersistedHistory+1, HISTORY_LIMIT );
		bool bReplaced = false;
		bool bSaved = SaveHistory ( m_dHistory, sError, bReplaced );
		if ( bReplaced )
			m_iUnpersistedHistory = 0;
		return bSaved;
	}

	void RefreshCompletions ( const std::string & sLine, bool bSuccessful ) override
	{
		if ( m_fnCompletionProvider && m_fnCompletionMatches && g_pAttemptedCompletionFunction )
		{
			if ( bSuccessful )
				RememberSessionWords ( sLine );
			m_dCompletionWords = m_fnCompletionProvider ( sLine );
		}
	}
};
#endif

std::unique_ptr<LineEditor_i> CreateLineEditor ( const CSphString & sDataDir, CSphString & sWarning, CompletionProvider_fn fnCompletionProvider )
{
#if !_WIN32
	g_sLocalDataDir = sDataDir;
	auto pEditor = std::make_unique<LocalLineEditor_c> ( std::move(fnCompletionProvider) );
	sWarning = pEditor->GetWarning();
	if ( !pEditor->IsActive() )
		return nullptr;
	return pEditor;
#else
	return nullptr;
#endif
}
}
