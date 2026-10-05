#ifndef _SREPO_PRIV_LOG_H
#define _SREPO_PRIV_LOG_H

#include <srutils/srepo/common.h>
#include <elog/elog.h>

#if defined(CONFIG_SREPO_LOG)

extern struct elog * srepo_logger;

extern void
srepo_vlog_conn(const sr_conn_ctx_t * __restrict connection,
                elog_severity                    severity,
                const char * __restrict          prefix,
                const char * __restrict          format,
                va_list                          args)
	__srepo_nonull(1, 3, 4) __printf(4, 0) __srepo_export;

extern void
srepo_vlog_sess(const sr_session_ctx_t * __restrict session,
                elog_severity                       severity,
                const char * __restrict             prefix,
                const char * __restrict             format,
                va_list                             args)
	__srepo_nonull(1, 3, 4) __printf(4, 0) __srepo_export;

#endif /* defined(CONFIG_SREPO_LOG) */

#endif /* _SREPO_PRIV_LOG_H */
