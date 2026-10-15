#include "srutils/srepo/log.h"

struct elog * srepo_logger = NULL;

static
void
srepo_log_fail(const char * prefix, int error)
{
	elog_log(srepo_logger,
	         ELOG_WARNING_SEVERITY,
	         "%s%slogging failure: %s.",
	         prefix,
	         prefix[0] ? ": " : "",
	         strerror(-error));
}

void
srepo_vlog(enum elog_severity      severity,
           const char * __restrict prefix,
           const char * __restrict format,
           va_list                 args)
{
	srepo_assert(srepo_logger);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * fmt;
	int    ret;

	ret = srepo_asprintf(&fmt,
	                     "%s%s%s.",
	                     prefix,
	                     prefix[0] ? ": " : "",
	                     format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		srepo_log_fail(prefix, ret);
}

void
srepo_vlog_conn(const sr_conn_ctx_t * __restrict connection,
                enum elog_severity               severity,
                const char * __restrict          prefix,
                const char * __restrict          format,
                va_list                          args)
{
	srepo_assert(srepo_logger);
	srepo_assert(connection);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * fmt;
	int    ret;

	ret = srepo_asprintf(&fmt,
	                     "%s[conn:%" PRIu32 "]: %s.",
	                     prefix,
	                     sr_get_cid((sr_conn_ctx_t *)connection),
	                     format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		srepo_log_fail(prefix, ret);
}

void
srepo_vlog_sess(const sr_session_ctx_t * __restrict session,
                enum elog_severity                  severity,
                const char * __restrict             prefix,
                const char * __restrict             format,
                va_list                             args)
{
	srepo_assert(srepo_logger);
	srepo_assert(session);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * fmt;
	int    ret;

	ret = srepo_asprintf(
		&fmt,
		"%s[conn:%" PRIu32 "|sess:%" PRIu32 "|ds:%s]: %s.",
		prefix,
		sr_get_cid(sr_session_get_connection((sr_session_ctx_t *)
		                                     session)),
		sr_session_get_id((sr_session_ctx_t *)session),
		srepo_dstore_str(sr_session_get_ds((sr_session_ctx_t *)
		                                   session)),
		format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		srepo_log_fail(prefix, ret);
}
