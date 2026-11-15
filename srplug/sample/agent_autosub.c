#include "agent_autosub.h"

static int
spl_agent_on_change(sr_session_ctx_t * session __unused,
                    uint32_t           sub_id __unused,
                    const char *       module __unused,
                    const char *       xpath __unused,
                    sr_event_t         event __unused,
                    uint32_t           request_id __unused,
                    void *             store __unused)
{
	/* Process change subscription... */

	return 0;
}

static const struct srplug_sub spl_agent_sub = {
	.kind   = SRPLUG_CHANGE_SUB_KIND,
	.change = {
		.module    = "asamplemodule",
		.xpath     = "/asamplemodule:container",
		.feature   = NULL,
		.on_change = spl_agent_on_change,
		.priority  = 0,
		.options   = SR_SUBSCR_DEFAULT
	}
};

SPL_AGENT_AUTOSUB(spl_agent_sub);

static SRPLUG_AGENT_AUTOSUB(spl_agent_do_autosub, spl)

int
spl_agent_autosub(struct srplug_agent * agent)
{
	return spl_agent_do_autosub(agent, NULL);
}
