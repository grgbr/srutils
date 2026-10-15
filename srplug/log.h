#ifndef _SRPLUG_INTERN_LOG_H
#define _SRPLUG_INTERN_LOG_H

#include "srutils/srplug/common.h"
#include "srutils/srepo/log.h"

#if defined(CONFIG_SREPO_LOG)

#define srplug_log(_svrt, _fmt, ...) \
	srepo_log(_svrt, "srplug", _fmt, ## __VA_ARGS__)

#define srplug_err(_fmt, ...) \
	srplug_log(ELOG_ERR_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_warn(_fmt, ...) \
	srplug_log(ELOG_WARNING_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_notice(_fmt, ...) \
	srplug_log(ELOG_NOTICE_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_info(_fmt, ...) \
	srplug_log(ELOG_INFO_SEVERITY, _fmt, ## __VA_ARGS__)

#if defined(CONFIG_SRPLUG_DEBUG)
#define srplug_debug(_fmt, ...) \
	srplug_log(ELOG_DEBUG_SEVERITY, _fmt, ## __VA_ARGS__)
#else  /* !defined(CONFIG_SRPLUG_DEBUG) */
#define srplug_debug(_fmt, ...)
#endif /* defined(CONFIG_SRPLUG_DEBUG) */


#define srplug_log_sess(_sess, _svrt, _fmt, ...) \
	srepo_log_sess(_sess, _svrt, "srplug", _fmt, ## __VA_ARGS__)

#define srplug_sess_err(_sess, _fmt, ...) \
	srplug_log_sess(_sess, ELOG_ERR_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_sess_warn(_sess, _fmt, ...) \
	srplug_log_sess(_sess, ELOG_WARNING_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_sess_notice(_sess, _fmt, ...) \
	srplug_log_sess(_sess, ELOG_NOTICE_SEVERITY, _fmt, ## __VA_ARGS__)

#define srplug_sess_info(_sess, _fmt, ...) \
	srplug_log_sess(_sess, ELOG_INFO_SEVERITY, _fmt, ## __VA_ARGS__)

#if defined(CONFIG_SRPLUG_DEBUG)
#define srplug_sess_debug(_sess, _fmt, ...) \
	srplug_log_sess(_sess, ELOG_DEBUG_SEVERITY, _fmt, ## __VA_ARGS__)
#else  /* !defined(CONFIG_SRPLUG_DEBUG) */
#define srplug_sess_debug(_sess, _fmt, ...)
#endif /* defined(CONFIG_SRPLUG_DEBUG) */


#define srplug_log_sess_node(_sess, _node, _svrt, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        _svrt, \
	                        "srplug", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#define srplug_sess_node_err(_sess, _node, _fmt, ...) \
	srplug_log_sess_node(_sess, \
	                     _node, \
	                     ELOG_ERR_SEVERITY, \
	                     _fmt, \
	                     ## __VA_ARGS__)

#define srplug_sess_node_warn(_sess, _node, _fmt, ...) \
	srplug_log_sess_node(_sess, \
	                     _node, \
	                     ELOG_WARNING_SEVERITY, \
	                     _fmt, \
	                     ## __VA_ARGS__)

#define srplug_sess_node_notice(_sess, _node, _fmt, ...) \
	srplug_log_sess_node(_sess, \
	                     _node, \
	                     ELOG_NOTICE_SEVERITY, \
	                     _fmt, \
	                     ## __VA_ARGS__)

#define srplug_sess_node_info(_sess, _node, _fmt, ...) \
	srplug_log_sess_node(_sess, \
	                     _node, \
	                     ELOG_INFO_SEVERITY, \
	                     _fmt, \
	                     ## __VA_ARGS__)

#if defined(CONFIG_SRPLUG_DEBUG)
#define srplug_sess_node_debug(_sess, _node, _fmt, ...) \
	srplug_log_sess_node(_sess, \
	                     _node, \
	                     ELOG_DEBUG_SEVERITY, \
	                     _fmt, \
	                     ## __VA_ARGS__)
#else  /* !defined(CONFIG_SRPLUG_DEBUG) */
#define srplug_sess_node_debug(_sess, _node, _fmt, ...)
#endif /* defined(CONFIG_SRPLUG_DEBUG) */

#else  /* !defined(CONFIG_SREPO_LOG) */

#define srplug_err(_fmt, ...)
#define srplug_warn(_fmt, ...)
#define srplug_notice(_fmt, ...)
#define srplug_info(_fmt, ...)
#define srplug_debug(_fmt, ...)

#define srplug_sess_err(_sess, _fmt, ...)
#define srplug_sess_warn(_sess, _fmt, ...)
#define srplug_sess_notice(_sess, _fmt, ...)
#define srplug_sess_info(_sess, _fmt, ...)
#define srplug_sess_debug(_sess, _fmt, ...)

#define srplug_sess_node_err(_sess, _node, _fmt, ...)
#define srplug_sess_node_warn(_sess, _node, _fmt, ...)
#define srplug_sess_node_notice(_sess, _node, _fmt, ...)
#define srplug_sess_node_info(_sess, _node, _fmt, ...)
#define srplug_sess_node_debug(_sess, _node, _fmt, ...)

#endif /* defined(CONFIG_SREPO_LOG) */

#endif /* _SRPLUG_INTERN_LOG_H */
