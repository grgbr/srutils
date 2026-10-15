#ifndef _SREPO_PRIV_XPATH_H
#define _SREPO_PRIV_XPATH_H

#include <srutils/srepo/log.h>

#if defined(CONFIG_SREPO_LOG)

extern void
srepo_xpath_vlog(const char * __restrict xpath,
                 enum elog_severity      severity,
                 const char * __restrict prefix,
                 const char * __restrict format,
                 va_list                 args)
	__srepo_nonull(1, 3, 4) __printf(4, 0) __srepo_export;

extern void
srepo_xpath_vlog_sess(const sr_session_ctx_t * __restrict session,
                      const char * __restrict             xpath,
                      enum elog_severity                  severity,
                      const char * __restrict             prefix,
                      const char * __restrict             format,
                      va_list                             args)
	__srepo_nonull(1, 2, 4, 5) __printf(5, 0) __srepo_export;

#endif /* defined(CONFIG_SREPO_LOG) */

#endif /* _SREPO_PRIV_XPATH_H */
