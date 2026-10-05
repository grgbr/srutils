#include "srutils/srepo/common.h"

int
srepo_asprintf(char ** string, const char * format, ...)
{
	srepo_assert(string);
	srepo_assert(format);

	va_list args;
	int     ret;

	va_start(args, format);
	ret = srepo_vasprintf(string, format, args);
	va_end(args);

	if (ret < 0) {
		if (errno == ENOMEM)
			abort();
		return -errno;
	}

	return ret;
}

sr_error_t
srepo_acquire_context(sr_session_ctx_t *     session,
                      const struct ly_ctx ** context)
{
	srepo_assert(session);
	srepo_assert(context);

	const struct ly_ctx * ctx;

	ctx = sr_session_acquire_context(session);
	if (!ctx) {
		const sr_error_info_t * einfo;
		int                     err __unused;

		err = sr_session_get_error(session, &einfo);
		srepo_assert(!err);
		srepo_assert(einfo->err->err_code != SR_ERR_OK);
		if (einfo->err->err_code == SR_ERR_NO_MEMORY)
			srepo_abort();

		srepo_sess_info(session,
		                "cannot acquire context: %s",
		                sr_strerror(einfo->err->err_code));

		return einfo->err->err_code;
	}

	*context = ctx;

	return SR_ERR_OK;
}

const char *
srepo_dstore_str(sr_datastore_t ds)
{
	switch (ds) {
	case SR_DS_RUNNING:
		return "running";
	case SR_DS_STARTUP:
		return "startup";
	case SR_DS_CANDIDATE:
		return "candidate";
	case SR_DS_OPERATIONAL:
		return "operational";
	case SR_DS_FACTORY_DEFAULT:
		return "factory-default";
	default:
		srepo_assert(0);
		return "??";
	}
}

sr_error_t
srepo_apply_changes(sr_session_ctx_t * session)
{
	srepo_assert(session);

	sr_error_t err;

	err = sr_apply_changes(session, 0);
	if (err == SR_ERR_OK)
		return err;

	srepo_assert(err != SR_ERR_INVAL_ARG);
	if (err == SR_ERR_NO_MEMORY)
		srepo_abort();

	srepo_sess_notice(session,
	                  "cannot apply changes: %s",
	                  sr_strerror(err));
	return err;
}

sr_error_t
srepo_replace_config(sr_session_ctx_t * session,
                     const char *       module,
                     struct lyd_node *  tree)
{
	srepo_assert(session);
	srepo_assert((sr_session_get_ds(session) == SR_DS_STARTUP) ||
	             (sr_session_get_ds(session) == SR_DS_RUNNING) ||
	             (sr_session_get_ds(session) == SR_DS_CANDIDATE));
	srepo_assert(module);
	srepo_assert(module[0]);
	srepo_assert(tree);

	int err;

	err = sr_replace_config(session, module, tree, 0);
	if (err == SR_ERR_OK)
		return SR_ERR_OK;

	srepo_assert(err != SR_ERR_INVAL_ARG);
	if (err == SR_ERR_NO_MEMORY)
		srepo_abort();

	srepo_sess_notice(session,
	                  "cannot replace configuration datastore: %s",
	                  sr_strerror(err));
	return err;
}
