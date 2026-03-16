//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "notifier.h"

#if _WIN32 || (defined(__MACH__) && defined(__APPLE__)) || defined(__FreeBSD__)

namespace sd
{

int status ( const char * ) noexcept { return 0; }
void statusf ( const char * , ... ) noexcept {}
void mainpid ( int ) noexcept {}
void ready() noexcept {}
void stopping() noexcept {};
void reloading() noexcept {};
void keep_alive() noexcept {};

[[nodiscard]] std::optional<uint64_t> WatchdogTimeout () noexcept {return std::nullopt;}

}

#else

#include "std/generics.h"
#include "searchdaemon.h"

#include <errno.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

class SocketWrapper_c
{
	int m_fd = -1;
	int m_iError = 0;

	int Initialize()
	{
		assert ( m_fd == -1 );
		const char* szSocket = getenv ( "NOTIFY_SOCKET" );
		if (!szSocket)
			return 0;

		// only UNIX socket - absolute path, or abstract socket
		if ( szSocket[0] != '/' && szSocket[0] != '@' )
			return -EAFNOSUPPORT;

		Str_t sSocket { FromSz ( szSocket ) };
		if ( sSocket.second >= sizeof(sockaddr_un::sun_path))
			return -E2BIG;

		struct sockaddr_un tAddress;
		::memcpy(tAddress.sun_path, sSocket.first, sSocket.second);

		// Support for abstract socket
		if (tAddress.sun_path[0] == '@')
			tAddress.sun_path[0] = 0;

		m_fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0); // cloexec broken on mac
		if  (m_fd < 0 )
			return -errno;

		const auto* pSockaddr = (const struct sockaddr *) &tAddress;
		if (::connect(m_fd, pSockaddr, offsetof(struct sockaddr_un, sun_path) + sSocket.second) != 0)
			return -errno;
		return 0;
	}

public:
	NONCOPYMOVABLE ( SocketWrapper_c );
	SocketWrapper_c()
	{
		m_iError = Initialize();
	}
	~SocketWrapper_c()
	{
		SafeCloseSocket (m_fd);
	}

	[[nodiscard]] bool IsValid() const { return m_iError==0; }
	[[nodiscard]] int GetError() const noexcept { return m_iError; }

	int SendMsg ( Str_t sMsg )
	{
		if (m_fd<0)
			return 0;

		assert (!IsEmpty(sMsg));

		const auto iWritten  = ::write ( m_fd, sMsg.first, sMsg.second );
		if (iWritten != sMsg.second)
			return iWritten < 0 ? -errno : -EPROTO;

		return 1; // means 'all is ok'
	}
};

int notify(Str_t sMessage) {
	if (IsEmpty(sMessage))
		return -EINVAL;

	SocketWrapper_c tSocket;
	if ( !tSocket.IsValid() )
		return tSocket.GetError();

	return tSocket.SendMsg ( sMessage );
}

int notifysz(const char *szMessage) {
	return notify (FromSz ( szMessage ));
}

void notifyf ( const char * sTemplate, ... ) noexcept
{
	StringBuilder_c sMessage;
	va_list ap;
	va_start ( ap, sTemplate );
	sMessage.vSprintf ( sTemplate, ap );
	va_end ( ap );
	::notify (Str_t(sMessage));
}

namespace sd
{

int status (const char *szMessage) noexcept
{
	StringBuilder_c sMessage;
	sMessage << "STATUS=" << szMessage;
	return ::notify (Str_t(sMessage));
}

void statusf ( const char * sTemplate, ... ) noexcept
{
	StringBuilder_c sMessage;
	sMessage << "STATUS=";
	va_list ap;
	va_start ( ap, sTemplate );
	sMessage.vSprintf ( sTemplate, ap );
	va_end ( ap );
	::notify (Str_t(sMessage));
}

void mainpid (int iPid) noexcept
{
	::notifyf ("MAINPID=%d", iPid);
}

void ready() noexcept
{
	::notify (FROMS("READY=1"));
}

void stopping() noexcept
{
	::notify (FROMS("STOPPING=1"));
}

void reloading() noexcept
{
	uint64_t now = MonoMicroTimer();
	::notifyf ( "RELOADING=1\nMONOTONIC_USEC=%U", now);
}

void keep_alive() noexcept
{
	::notify (FROMS("WATCHDOG=1"));
}

[[nodiscard]] std::optional<uint64_t> WatchdogTimeout () noexcept
{
	auto tTimer = env_ulong ("WATCHDOG_USEC");
	if (!tTimer.has_value())
		return std::nullopt;

	auto tPid = env_long ("WATCHDOG_PID");
	if (tPid.has_value() && tPid.value()!=getpid() )
		return std::nullopt;

	return tTimer;
}

}

#endif

/*
 Well-known assignments
The following assignments have a defined meaning:

READY=1
Tells the service manager that service startup is finished, or the service finished re-loading its configuration. This is only used by systemd if the service definition file has Type=notify or Type=notify-reload set. Since there is little value in signaling non-readiness, the only value services should send is "READY=1" (i.e. "READY=0" is not defined).

RELOADING=1
Tells the service manager that the service is beginning to reload its configuration. This is useful to allow the service manager to track the service's internal state, and present it to the user. Note that a service that sends this notification must also send a "READY=1" notification when it completed reloading its configuration. Reloads the service manager is notified about with this mechanisms are propagated in the same way as they are when originally initiated through the service manager. This message is particularly relevant for Type=notify-reload services, to inform the service manager that the request to reload the service has been received and is now being processed.

Added in version 217.

STOPPING=1
Tells the service manager that the service is beginning its shutdown. This is useful to allow the service manager to track the service's internal state, and present it to the user.

Added in version 217.

MONOTONIC_USEC=…
A field carrying the monotonic timestamp (as per CLOCK_MONOTONIC) formatted in decimal in μs, when the notification message was generated by the client. This is typically used in combination with "RELOADING=1", to allow the service manager to properly synchronize reload cycles. See systemd.service(5) for details, specifically "Type=notify-reload".

Added in version 253.

STATUS=…
Passes a single-line UTF-8 status string back to the service manager that describes the service state. This is free-form and can be used for various purposes: general state feedback, fsck-like programs could pass completion percentages and failing programs could pass a human-readable error message. Example: "STATUS=Completed 66% of file system check…"

Added in version 233.

NOTIFYACCESS=…
Reset the access to the service status notification socket during runtime, overriding NotifyAccess= setting in the service unit file. See systemd.service(5) for details, specifically "NotifyAccess=" for a list of accepted values.

Added in version 254.

ERRNO=…
If a service fails, the errno-style error code, formatted as string. Example: "ERRNO=2" for ENOENT.

Added in version 233.

BUSERROR=…
If a service fails, the D-Bus error-style error code. Example: "BUSERROR=org.freedesktop.DBus.Error.TimedOut".

Added in version 233.

VARLINKERROR=…
If a service fails, the Varlink error-style error code. Example: "VARLINKERROR=org.varlink.service.InvalidParameter".

Added in version 257.

EXIT_STATUS=…
The exit status of a service or the manager itself. Note that systemd currently does not consume this value when sent by services, so this assignment is only informational. The manager will send this notification to its notification socket, which may be used to collect an exit status from the system (a container or VM) as it shuts down. For example, mkosi(1) makes use of this. The value to return may be set via the systemctl(1) exit verb.

Added in version 254.

MAINPID=…
Change the main process ID (PID) of the service. This is especially useful in the case where the real main process is not directly forked off by the service manager. Example: "MAINPID=4711".

Added in version 233.

MAINPIDFDID=…
The pidfd inode number of the new main process (specified through MAINPID=). This information can be acquired through sd_pidfd_get_inode_id(3) on the pidfd and is used to identify the process in a race-free fashion. Alternatively, a pidfd can be sent directly to the service manager (see MAINPIDFD=1 below).

Added in version 257.

MAINPIDFD=1
Similar to MAINPID= with MAINPIDFDID=, but the process is referenced directly by the pidfd passed to the service manager. This is useful if pidfd id is not supported on the system. Exactly one fd is expected for this notification.

Added in version 257.

WATCHDOG=1
Tells the service manager to update the watchdog timestamp. This is the keep-alive ping that services need to issue in regular intervals if WatchdogSec= is enabled for it. See systemd.service(5) for information how to enable this functionality and sd_watchdog_enabled(3) for the details of how the service can check whether the watchdog is enabled.

WATCHDOG=trigger
Tells the service manager that the service detected an internal error that should be handled by the configured watchdog options. This will trigger the same behaviour as if WatchdogSec= is enabled and the service did not send "WATCHDOG=1" in time. Note that WatchdogSec= does not need to be enabled for "WATCHDOG=trigger" to trigger the watchdog action. See systemd.service(5) for information about the watchdog behavior.

Added in version 243.

WATCHDOG_USEC=…
Reset watchdog_usec value during runtime. Notice that this is not available when using sd_event_set_watchdog() or sd_watchdog_enabled(). Example : "WATCHDOG_USEC=20000000"

Added in version 233.

EXTEND_TIMEOUT_USEC=…
Tells the service manager to extend the startup, runtime or shutdown service timeout corresponding the current state. The value specified is a time in microseconds during which the service must send a new message. A service timeout will occur if the message is not received, but only if the runtime of the current state is beyond the original maximum times of TimeoutStartSec=, RuntimeMaxSec=, and TimeoutStopSec=. See systemd.service(5) for effects on the service timeouts.

Added in version 236.

RESTART_RESET=1
Reset the restart counter of the service, which has the effect of restoring the restart duration to RestartSec= if RestartSteps= and RestartMaxDelaySec= are in use. For more information, refer to systemd.service(5).

Added in version 258.
 */