#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "sphinx.h"

static int read_timeout = 5;
static int children_count = 0;
static int max_children = 0;
static int sock;

int createServerSocket_IP(int port)
{
	int sock;
	static struct sockaddr_in iaddr;

	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	iaddr.sin_port = htons(port);
	fprintf(stderr, "INFO: creating a server socket on port %d\n", port);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "FATAL: unable to create socket on port %d\n", port);
		exit(1);
	}
	while (bind(sock, (struct sockaddr *)&iaddr,
		sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "INFO: failed to bind on port %d, retrying...\n", port);
		sleep(1);
	}
	return sock;
}

void sigchld(int arg)
{
	signal(SIGCHLD, sigchld);
	while (waitpid(0, (int *)0, WNOHANG | WUNTRACED) > 0)
		children_count--;
}

void sigterm(int arg)
{
	fprintf(stderr, "INFO: caught SIGTERM, shutting down\n");
	close(sock);
	exit(0);
}

int select_fd(int fd, int maxtime, int writep)
{
	fd_set fds, exceptfds;
	struct timeval read_timeout;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	FD_ZERO(&exceptfds);
	FD_SET(fd, &exceptfds);
	read_timeout.tv_sec = maxtime;
	read_timeout.tv_usec = 0;
	return select (fd + 1, writep ? NULL : &fds, writep ? &fds : NULL,
		&exceptfds, &read_timeout);
}

int iread(int fd, void *buf, int len)
{
	int res;

	do {
		if (read_timeout) {
			do {
				res = select_fd (fd, read_timeout, 0);
			} while (res == -1 && errno == EINTR);
			if (res <= 0) {
				if (res == 0) errno = ETIMEDOUT;
				return -1;
			}
		}
		res = read(fd, buf, len);
	}
	while (res == -1 && errno == EINTR);

	return res;
}

int main(int argc, char **argv)
{
	CSphIndex_VLN *idx;
	CSphQueryResult *r;
	int i, rsock, start = 0, count = 10, rcount, log, port;
	struct sockaddr_in remote_iaddr;
	socklen_t len;
	char query[1024], buf[2048], tbuf[26], *logname;
	time_t now;
	CSphConfig conf;
	CSphHash *confSearchd, *confCommon;

	// configure
	const char * sConfName = "sphinx.conf";
	struct stat tStat;
	if ( argc>1 )
	{
		if ( !stat ( argv[1], &tStat ) )
			sConfName = argv[1];
		else
			fprintf ( stderr, "WARNING: can not stat config file '%s', using default 'sphinx.conf'.\n", argv[1] );
	}

	fprintf ( stdout, "using config file '%s'...\n", sConfName );
	if ( !conf.open ( sConfName ) )
	{
		fprintf ( stderr, "FATAL: unable to open config file '%s'.\n", sConfName );
		return 1;
	}
	confCommon = conf.loadSection ( "common" );
	confSearchd = conf.loadSection ( "searchd" );

	#define CHECK_CONF(hash, key) \
		if (!hash->get(key)) { \
			fprintf(stderr, "FATAL: '%s' is not specified in config file\n", key); \
			return 1; \
		}

	CHECK_CONF(confCommon,  "index_path");
	port = confSearchd->get("port")
		? atoi(confSearchd->get("port"))
		: 0;
	if (!port) {
		fprintf(stderr, "FATAL: 'port' is not configured or invalid\n");
		return 1;
	}
	logname = confSearchd->get("log");
	if (!logname) logname = sphDup("searchd.log");
	if (confSearchd->get("read_timeout")) {
		i = atoi(confSearchd->get("read_timeout"));
		if (i >= 0) read_timeout = i;
	}
	if (confSearchd->get("max_children")) {
		i = atoi(confSearchd->get("max_children"));
		max_children = (i >= 0) ? i : 0;
	}

	DWORD iMorph = SPH_MORPH_NONE;
	char * pMorph = confCommon->get ( "morphology" );
	if ( pMorph )
	{
		if ( !strcmp ( pMorph, "stem_en" ) )
			iMorph = SPH_MORPH_STEM_EN;
		else if ( !strcmp ( pMorph, "stem_ru" ) )
			iMorph = SPH_MORPH_STEM_RU;
		else if ( !strcmp ( pMorph, "stem_enru" ) )
			iMorph = SPH_MORPH_STEM_EN | SPH_MORPH_STEM_RU;
		else
		{
			fprintf ( stderr, "WARNING: unknown morphology type '%s' ignored.\n", pMorph );
		}
	}

	// *** STARTUP ***

	idx = new CSphIndex_VLN(confCommon->get("index_path"));
	sock = createServerSocket_IP(port);
	listen(sock, 5);
	signal(SIGCHLD, sigchld);
	signal(SIGTERM, sigterm);
	if ((log = open(logname, O_CREAT | O_RDWR | O_APPEND,
		S_IREAD | S_IWRITE)) < 0)
	{
		fprintf(stderr, "FATAL: unable to append to '%s'", logname);
		exit(1);
	}

	// *** SERVE CLIENTS ***

	fprintf(stderr, "INFO: accepting connections\n");
	while (1) {
		len = sizeof(remote_iaddr);
		rsock = accept(sock, (struct sockaddr*)&remote_iaddr, &len);
		if (rsock < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			continue;
		if (rsock < 0) {
			fprintf(stderr, "FATAL: accept() failed (reason: %s)", strerror(errno));
			exit(1);
		}

		if (max_children && children_count >= max_children) {
			strcpy(buf, "RETRY Server maxed out\n");
			write(rsock, buf, strlen(buf));
			close(rsock);
			continue;
		}

		switch (fork()) 
		{
			// fork() failed
			case -1: exit(1); // FIXME: fork() failed, should log

			// child process, handle client
			case 0:
				while (1) {
					// read query
					if (iread(rsock, &start, 4) != 4) break;
					if (iread(rsock, &count, 4) != 4) break;
					if (iread(rsock, &i, 4) != 4) break;
					i = min(i, (int)sizeof(query)-1);
					if (iread(rsock, query, i) != i) break;
					query[i] = '\0';

					// read weights
					int iUserWeights;
					int dUserWeights [ SPH_MAX_FIELD_COUNT ];

					if ( iread ( rsock, &iUserWeights, 4 ) != 4 ) break;
					if ( iUserWeights<0 || iUserWeights>SPH_MAX_FIELD_COUNT ) break;
					if ( iUserWeights )
						if ( iread ( rsock, dUserWeights, iUserWeights*4 ) != iUserWeights*4 ) break;

					// execute it and log
					r = idx->query ( new CSphDict_CRC32 ( iMorph ), query, dUserWeights, iUserWeights );
					time(&now);
					ctime_r(&now, tbuf);
					tbuf[24] = '\0';
					sprintf(buf, "[%s] %d.%02d sec: [%d %d] %s\n",
						tbuf,
						r->queryTime / 1000000,
						(r->queryTime % 1000000) / 10000,
						start, count,
						query);
					flock(log, LOCK_EX);
					lseek(log, 0, SEEK_END);
					write(log, buf, strlen(buf));
					flock(log, LOCK_UN);

					// serve the answer to client
					rcount = min(count, r->matches->count - start);
					sprintf(buf, "MATCHES %d\n", rcount);
					write(rsock, buf, strlen(buf));
					for ( i=start; i<start+rcount; i++ )
					{
						sprintf ( buf, "MATCH %d %d %d\n",
							r->matches->data[i].m_iGroupID,
							r->matches->data[i].m_iDocID,
							r->matches->data[i].m_iWeight );
						write ( rsock, buf, strlen ( buf ) );
					}
					sprintf(buf, "TOTAL %d\n", r->matches->count);
					write(rsock, buf, strlen(buf));
					sprintf(buf, "TIME %d.%02d\n",
						r->queryTime / 1000000,
						(r->queryTime % 1000000) / 10000);
					write(rsock, buf, strlen(buf));
					sprintf(buf, "WORDS %d\n", r->numWords);
					write(rsock, buf, strlen(buf));
					for (i = 0; i < r->numWords; i++) {
						sprintf(buf, "WORD %s %d %d\n", r->wordStats[i].word,
						r->wordStats[i].docs, r->wordStats[i].hits);
						write(rsock, buf, strlen(buf));
					}
					break;
				}
				// bail out
				close(rsock);
				exit(0);
				break;

			// parent process, continue accept()ing
			default:
				children_count++;
				close(rsock);
				break;
		}
	}
}
