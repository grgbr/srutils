#ifndef _SREPO_COMMON_H
#define _SREPO_COMMON_H

#include <srutils/srepo/priv/common.h>
#include <sysrepo.h>
#include <stdlib.h>

static inline __srepo_nonull(1, 2) __printf(2, 0) __warn_result
int
srepo_vasprintf(char ** __restrict      string,
                const char * __restrict format,
                va_list                 args)
{
	srepo_assert(string);
	srepo_assert(format);

	int ret;

	ret = vasprintf(string, format, args);
	if (ret < 0) {
		if (errno == ENOMEM)
			abort();
	}

	return ret;
}

extern int
srepo_asprintf(char ** __restrict string, const char * __restrict format, ...)
	__srepo_nonull(1, 2) __printf(2, 3) __warn_result __srepo_export;

extern sr_error_t
srepo_acquire_context(sr_session_ctx_t *     session,
                      const struct ly_ctx ** context)
	__srepo_nonull(1, 2) __srepo_export;

static inline __srepo_nonull(1)
void
srepo_release_context(sr_session_ctx_t * session)
{
	srepo_assert(session);

	sr_session_release_context(session);
}

extern const char *
srepo_dstore_str(sr_datastore_t ds)
	__srepo_export;

static inline __srepo_nonull(1)
void
srepo_switch_dstore(sr_session_ctx_t * session, sr_datastore_t dstore)
{
	srepo_assert(session);

	sr_error_t ret __unused;

	ret = sr_session_switch_ds(session, dstore);
	srepo_assert(ret == SR_ERR_OK);
}

extern sr_error_t
srepo_apply_changes(sr_session_ctx_t * session)
	__srepo_nonull(1) __warn_result;

/*
 * Replace an entire sysrepo configuration datastore with the data tree given in
 * argument.
 *
 * Datastore currently attached to @p session session MUST be either
 * `SR_DS_STARTUP`, `SR_DS_RUNNING` or `SR_DS_CANDIDATE`.
 *
 * In addition, the @p tree given in argument MUST have been created using the
 * YANG context related to the session given in argument.
 *
 * Note that `tree' data tree will be freed once this function call has
 * returned.
 */
extern sr_error_t
srepo_replace_config(sr_session_ctx_t * session,
                     const char *       module,
                     struct lyd_node *  tree)
	__srepo_nonull(1, 2, 3) __warn_result;

#endif /* _SREPO_COMMON_H */
