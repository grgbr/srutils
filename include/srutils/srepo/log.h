#ifndef _SREPO_LOG_H
#define _SREPO_LOG_H

#include <srutils/srepo/priv/log.h>

#if defined(CONFIG_SREPO_LOG)

static inline __srepo_nonull(1, 3, 4) __printf(4, 5)
void
srepo_log_conn(const sr_conn_ctx_t * __restrict connection,
               elog_severity                    severity,
               const char * __restrict          prefix,
               const char * __restrict          format,
               ...)
{
	srepo_assert(connection);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	if (srepo_logger) {
		va_list args;

		va_start(args, format);
		srepo_vlog_conn(connection, severity, prefix, format, args);
		va_end(args);
	}
}

#define srepo_conn_err(_conn, _fmt, ...) \
	srepo_log_conn(_conn, \
	               ELOG_ERR_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_conn_warn(_conn, _fmt, ...) \
	srepo_log_conn(_conn, \
	               ELOG_WARNING_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_conn_notice(_conn, _fmt, ...) \
	srepo_log_conn(_conn, \
	               ELOG_NOTICE_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_conn_info(_conn, _fmt, ...) \
	srepo_log_conn(_conn, \
	               ELOG_INFO_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#if defined(CONFIG_SREPO_DEBUG)

#define srepo_conn_debug(_conn, _fmt, ...) \
	srepo_log_conn(_conn, \
	               ELOG_DEBUG_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#endif /* defined(CONFIG_SREPO_DEBUG) */

static inline __srepo_nonull(1, 3, 4) __printf(4, 5)
void
srepo_log_sess(const sr_session_ctx_t * __restrict session,
               elog_severity                       severity,
               const char * __restrict             prefix,
               const char * __restrict             format,
               ...)
{
	srepo_assert(session);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	if (srepo_logger) {
		va_list args;

		va_start(args, format);
		srepo_vlog_sess(session, severity, format, args);
		va_end(args);
	}
}

#define srepo_sess_err(_sess, _fmt, ...) \
	srepo_log_sess(_sess, \
	               ELOG_ERR_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_sess_warn(_sess, _fmt, ...) \
	srepo_log_sess(_sess, \
	               ELOG_WARNING_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_sess_notice(_sess, _fmt, ...) \
	srepo_log_sess(_sess, \
	               ELOG_NOTICE_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#define srepo_sess_info(_sess, _fmt, ...) \
	srepo_log_sess(_sess, \
	               ELOG_INFO_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#if defined(CONFIG_SREPO_DEBUG)

#define srepo_sess_debug(_sess, _fmt, ...) \
	srepo_log_sess(_sess, \
	               ELOG_DEBUG_SEVERITY, \
	               "srepo", \
	               _fmt, \
	               ## __VA_ARGS__)

#endif /* defined(CONFIG_SREPO_DEBUG) */

static inline
void
srepo_log_setup(struct elog * logger)
{
	srepo_logger = logger;
}

#else  /* !defined(CONFIG_SREPO_LOG) */

#define srepo_conn_err(_conn, _fmt, ...)
#define srepo_conn_warn(_conn, _fmt, ...)
#define srepo_conn_notice(_conn, _fmt, ...)
#define srepo_conn_info(_conn, _fmt, ...)
#if defined(CONFIG_SREPO_DEBUG)
#define srepo_conn_debug(_conn, _fmt, ...)
#endif /* defined(CONFIG_SREPO_DEBUG) */

#define srepo_sess_err(_sess, _fmt, ...)
#define srepo_sess_warn(_sess, _fmt, ...)
#define srepo_sess_notice(_sess, _fmt, ...)
#define srepo_sess_info(_sess, _fmt, ...)
#if defined(CONFIG_SREPO_DEBUG)
#define srepo_sess_debug(_sess, _fmt, ...)
#endif /* defined(CONFIG_SREPO_DEBUG) */

static inline
void
srepo_log_setup(struct elog * logger __unused)
{
}

#endif /* defined(CONFIG_SREPO_LOG) */

#endif /* _SREPO_LOG_H */
