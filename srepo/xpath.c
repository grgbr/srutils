#include "srutils/srepo/xpath.h"

ssize_t
srepo_xpath_concat(char * __restrict destination,
                   size_t            destination_length,
                   char * __restrict source,
                   size_t            source_length)
{
	srepo_assert(destination);
	srepo_assert(destination_length < SREPO_XPATH_SIZE);
	srepo_assert(source);
	srepo_assert(source_length < SREPO_XPATH_SIZE);

	size_t len = destination_length + source_length;

	if (len) {
		if (len < SREPO_XPATH_SIZE) {
			memcpy(&destination[destination_length],
			       source,
			       source_length);
			destination[destination_length + source_length] = '\0';

			return len;
		}
		else
			return -ENAMETOOLONG;
	}
	else
		return -ENODATA;
}

ssize_t
srepo_xpath_vprintf(char * __restrict       xpath,
                    size_t                  length,
                    const char * __restrict format,
                    va_list                 args)
{
	srepo_assert(xpath);
	srepo_assert((length + 1) < SREPO_XPATH_SIZE);
	srepo_assert(length == strnlen(xpath, SREPO_XPATH_SIZE));
	srepo_assert(format);
	srepo_assert(format[0]);

	size_t sz = SREPO_XPATH_SIZE - length;
	int    len;

	len = vsnprintf(&xpath[length],
	                sz,
	                format,
	                args);
	srepo_assert(len);
	if (len < 0)
		return -errno;
	else if ((size_t)len >= sz)
		return -ENAMETOOLONG;

	return (ssize_t)(length + len);
}

ssize_t
srepo_xpath_printf(char * __restrict       xpath,
                   size_t                  length,
                   const char * __restrict format,
                   ...)
{
	srepo_assert(xpath);
	srepo_assert((length + 1) < SREPO_XPATH_SIZE);
	srepo_assert(length == strnlen(xpath, SREPO_XPATH_SIZE));
	srepo_assert(format);
	srepo_assert(format[0]);

	va_list args;
	size_t  sz = SREPO_XPATH_SIZE - length;
	int     len;

	va_start(args, format);
	/*
	 * Hide the following warning since format string and arguments should
	 * have been checked thanks to the `__printf' attribute given to this
	 * function declaration.
	 */
STROLL_IGNORE_WARN("-Wformat-nonliteral")
	len = snprintf(&xpath[length], sz, format, args);
STROLL_RESTORE_WARN
	va_end(args);
	srepo_assert(len);
	if (len < 0)
		return -errno;
	else if ((size_t)len >= sz)
		return -ENAMETOOLONG;

	return (ssize_t)(length + len);
}

char *
srepo_xpath_create(const char * __restrict xpath, size_t length)
{
	srepo_assert(xpath);
	srepo_assert(length < SREPO_XPATH_SIZE);
	srepo_assert(length == strnlen(xpath, SREPO_XPATH_SIZE));

	char * pth;

	pth = srepo_xpath_alloc();
	if (length)
		memcpy(pth, xpath, length);
	pth[length] = '\0';

	return pth;
}

ssize_t
srepo_xpath_vcreatef(char ** __restrict      xpath,
                     const char * __restrict format,
                     va_list                 args)
{
	srepo_assert(xpath);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * pth;
	int    len;

	pth = srepo_xpath_alloc();

	len = srepo_xpath_vprintf(pth, 0, format, args);
	if (len >= 0) {
		*xpath = pth;
		return (ssize_t)len;
	}

	srepo_free(pth);

	return len;
}

/******************************************************************************
 * XPATH logging.
 ******************************************************************************/

#if defined(CONFIG_SREPO_LOG)

void
srepo_xpath_vlog(const char * __restrict xpath,
                 enum elog_severity      severity,
                 const char * __restrict prefix,
                 const char * __restrict format,
                 va_list                 args)
{
	srepo_assert(srepo_logger);
	srepo_assert(srepo_xpath_validate(xpath) > 0);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * fmt;
	int    ret;

	ret = srepo_asprintf( &fmt, "%s[path:%s]: %s.", prefix, xpath, format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		elog_log(srepo_logger,
		         ELOG_WARNING_SEVERITY,
		         "%s%slogging failure: %s.",
		         prefix,
		         prefix[0] ? ": " : "",
		         strerror(-ret));
}

void
srepo_xpath_vlog_sess(const sr_session_ctx_t * __restrict session,
                      const char * __restrict             xpath,
                      enum elog_severity                  severity,
                      const char * __restrict             prefix,
                      const char * __restrict             format,
                      va_list                             args)
{
	srepo_assert(srepo_logger);
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(xpath) > 0);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * fmt;
	int    ret;

	ret = srepo_asprintf(
		&fmt,
		"%s[conn:%" PRIu32 "|sess:%" PRIu32 "|ds:%s|path:%s]: %s.",
		prefix,
		sr_get_cid(sr_session_get_connection((sr_session_ctx_t *)
		                                     session)),
		sr_session_get_id((sr_session_ctx_t *)session),
		srepo_dstore_str(sr_session_get_ds((sr_session_ctx_t *)
		                                   session)),
		xpath,
		format);
	if (ret > 0) {
		elog_vlog(srepo_logger, severity, fmt, args);
		srepo_free(fmt);
	}
	else
		elog_log(srepo_logger,
		         ELOG_WARNING_SEVERITY,
		         "%s%slogging failure: %s.",
		         prefix,
		         prefix[0] ? ": " : "",
		         strerror(-ret));
}

#endif /* defined(CONFIG_SREPO_LOG) */
