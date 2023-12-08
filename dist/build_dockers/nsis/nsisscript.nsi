; The name of the installer
Name "Manticore Search"

; The file to write
OutFile "manticore_installer.exe"

; Request application privileges for Windows Vista and higher
;RequestExecutionLevel admin

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir $PROGRAMFILES\Manticore

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "SOFTWARE\Manticore Software LTD" "manticore"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

Function unpackInstall
	Pop $R0  ; txt file name
	Pop $R1  ; archive name
	FileOpen $0 $INSTDIR\$R0 r
	IfErrors Fail1
	FileRead $0 $1
	DetailPrint $1
	FileClose $0
	NSISdl::download $1 $R1
	Pop $0
	StrCmp $0 success success1
	SetDetailsView show
	DetailPrint "Error! Unable to download $R1!"
	Abort

success1:
	Delete $INSTDIR\$R0
	nsisunz::Unzip "$INSTDIR\$R1" "$INSTDIR"
	Pop $0
	StrCmp $0 success success2
	DetailPrint "Error! Unable to unzip $R1!"

success2:
	Delete $INSTDIR\$R1
	Goto Ok

Fail1:
	MessageBox MB_OK "Error! Unable to read $R0!"
	Abort

Ok:
FunctionEnd

Function createConfig
	StrCpy $R0 "$INSTDIR\etc\manticoresearch\manticore.conf"
	FileOpen $0 $R0 w
	FileWrite $0 "searchd$\n"
	FileWrite $0 "{$\n"
	FileWrite $0 "    listen = 127.0.0.1:9312$\n"
	FileWrite $0 "    listen = 127.0.0.1:9306:mysql$\n"
	FileWrite $0 "    listen = 127.0.0.1:9308:http$\n"
	FileWrite $0 "    log = $INSTDIR/var/log/manticore/searchd.log$\n"
	FileWrite $0 "    query_log = $INSTDIR/var/log/manticore/query.log$\n"
	FileWrite $0 "    pid_file = $INSTDIR/var/run/manticore/searchd.pid$\n"
	FileWrite $0 "    data_dir = $INSTDIR/var/data$\n"
	FileWrite $0 "    query_log_format = sphinxql$\n"
	FileWrite $0 "}$\n"
	IfErrors Fail1
	Goto Ok

Fail1:
	MessageBox MB_OK "Error! Unable to write to $R0!"
	Abort

Ok:
FunctionEnd

Section "Manticore Search"
	SectionIn RO
	SetOutPath $INSTDIR

	File "manticore_src.txt"
	Push "manticore.zip"
	Push "manticore_src.txt"
	Call unpackInstall

	File "tzdata_src.txt"
	Push "tzdata.zip"
	Push "tzdata_src.txt"
	Call unpackInstall

	CreateDirectory "$INSTDIR\var\data"
	CreateDirectory "$INSTDIR\var\log\manticore"
	CreateDirectory "$INSTDIR\var\run\manticore"

	Call createConfig

	; Write the installation path into the registry
	WriteRegStr HKLM "SOFTWARE\Manticore Software LTD" "manticore" "$INSTDIR"

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "DisplayName" "Manticore Search"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore" "NoRepair" 1

	WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Manticore Executor"
	; Check if Docker is installed and working
	nsExec::ExecToStack 'docker --version'
	Pop $0  ; Pop the exit code from the stack
	StrCmp $0 0 docker_installed  ; Compare the exit code to 0 (success)
	MessageBox MB_OK "Docker is not installed or not working correctly. Please ensure Docker is installed and try again."
	Abort

	docker_installed:
	Pop $1  ; Pop the command output from the stack (and ignore it)

	; Pull Docker image from the URL in executor_src.txt
	; Var hFile
	; Var R0
	FileOpen $0 "executor_src.txt" r
	FileRead $0 $R0
	FileClose $0
	nsExec::Exec '"docker" "pull" $R0'
	Pop $0  ; Pop the exit code from the stack
	StrCmp $0 0 docker_pull_succeed  ; Compare the exit code to 0 (success)
	MessageBox MB_OK "Failed to pull Docker image from: $R0"
	Abort

	docker_pull_succeed:

	File "buddy_src.txt"
	Push "buddy.zip"
	Push "buddy_src.txt"
	Call unpackInstall
SectionEnd

Section "Manticore Columnar Library"
	File "mcl_src.txt"
	Push "mcl.zip"
	Push "mcl_src.txt"
	Call unpackInstall
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Manticore"
	DeleteRegKey HKLM "SOFTWARE\Manticore Software LTD"

	; Remove files and uninstaller
	RmDir /r $INSTDIR\bin
	RmDir /r $INSTDIR\etc
	RmDir /r $INSTDIR\include
	RmDir /r $INSTDIR\share
	RmDir /r $INSTDIR\__MACOSX
	Delete $INSTDIR\uninstall.exe

	; Remove directories
	RMDir "$SMPROGRAMS\Manticore"
	RMDir "$INSTDIR"

SectionEnd
