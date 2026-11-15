#ifndef _SRUTILS_SRPLUG_SAMPLE_DAEMON_AUTOSUB_H
#define _SRUTILS_SRPLUG_SAMPLE_DAEMON_AUTOSUB_H

#include <srutils/srplug/daemon.h>

#define SPL_DAEMON_AUTOSUB(_sub) \
	SRPLUG_AUTOSUB(_sub, spl);

extern srplug_daemon_autosubscribe_fn spl_daemon_autosub;

#endif /* _SRUTILS_SRPLUG_SAMPLE_DAEMON_AUTOSUB_H */
