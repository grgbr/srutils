#ifndef _SRUTILS_SRPLUG_SAMPLE_AGENT_AUTOSUB_H
#define _SRUTILS_SRPLUG_SAMPLE_AGENT_AUTOSUB_H

#include <srutils/srplug/agent.h>

#define SPL_AGENT_AUTOSUB(_sub) \
	SRPLUG_AUTOSUB(_sub, spl);

extern int spl_agent_autosub(struct srplug_agent * agent);

#endif /* _SRUTILS_SRPLUG_SAMPLE_AGENT_AUTOSUB_H */
