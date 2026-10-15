#include "srutils/srepo/log.h"

struct elog * srepo_logger = NULL;

void
srepo_vlog_conn(const sr_conn_ctx_t * __restrict connection,
                elog_severity                    severity,
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
	                     sr_get_cid(connection),
	                     format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		elog_vlog(srepo_logger,
		          ELOG_WARNING_SEVERITY,
		          "%s: logging failure: %s.",
		          prefix,
		          strerror(-ret));
}

void
srepo_vlog_sess(const sr_session_ctx_t * __restrict session,
                elog_severity                       severity,
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
		sr_get_cid(sr_session_get_connection(session)),
		sr_session_get_id(session),
		srepo_dstore_str(sr_session_get_ds(session)),
		format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		elog_vlog(srepo_logger,
		          ELOG_WARNING_SEVERITY,
		          "%s: logging failure: %s.",
		          prefix,
		          strerror(-ret));
}
