#include "daemon_autosub.h"

static int
spl_daemon_on_change(sr_session_ctx_t * session __unused,
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

static const struct srplug_sub spl_daemon_sub0 = {
	.kind   = SRPLUG_CHANGE_SUB_KIND,
	.change = {
		.module    = "asamplemodule",
		.xpath     = "/asamplemodule:container0",
		.feature   = NULL,
		.on_change = spl_daemon_on_change,
		.priority  = 0,
		.options   = SR_SUBSCR_DEFAULT
	}
};

SPL_DAEMON_AUTOSUB(spl_daemon_sub0);

static const struct srplug_sub spl_daemon_sub1 = {
	.kind   = SRPLUG_CHANGE_SUB_KIND,
	.change = {
		.module    = "asamplemodule",
		.xpath     = "/asamplemodule:container1",
		.feature   = NULL,
		.on_change = spl_daemon_on_change,
		.priority  = 0,
		.options   = SR_SUBSCR_DEFAULT
	}
};

SPL_DAEMON_AUTOSUB(spl_daemon_sub1);

SRPLUG_DAEMON_AUTOSUB(spl_daemon_autosub, spl)
