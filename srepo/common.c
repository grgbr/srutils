#include "common.h"
#include <srutils/srepo/xpath.h>

sr_error_t
srepo_ly_error(LY_ERR error)
{
	switch (error) {
	case LY_SUCCESS:
		return SR_ERR_OK;
	case LY_EMEM:
		return SR_ERR_NO_MEMORY;
	case LY_ESYS:
		return SR_ERR_SYS;
	case LY_EINVAL:
		return SR_ERR_INVAL_ARG;
	case LY_EEXIST:
		return SR_ERR_EXISTS;
	case LY_ENOTFOUND:
		return SR_ERR_NOT_FOUND;
	case LY_EVALID:
		return SR_ERR_VALIDATION_FAILED;
	case LY_EDENIED:
		return SR_ERR_OPERATION_FAILED;
	case LY_EINT:
	case LY_EINCOMPLETE:
	case LY_ERECOMPILE:
	case LY_ENOT:
	case LY_EOTHER:
	case LY_EPLUGIN:
		return SR_ERR_LY;
	default:
		srepo_assert(0);
		return SR_ERR_LY;
	}
}

sr_error_t
srepo_sys_error(int error)
{
	switch (error) {
	case 0:
		return SR_ERR_OK;
	case -ENODEV:
	case -ENOENT:
		return SR_ERR_NOT_FOUND;
	case -EINVAL:
	case -ENODATA:
	case -ENAMETOOLONG:
		return SR_ERR_INVAL_ARG;
	case -ENOTSUP:
		return SR_ERR_UNSUPPORTED;
	case -EPERM:
		return SR_ERR_OPERATION_FAILED;
	case -EACCES:
		return SR_ERR_UNAUTHORIZED;
	case -ETIME:
	case -ETIMEDOUT:
		return SR_ERR_TIME_OUT;
	case -ENOLCK:
	case -EDEADLOCK:
		return SR_ERR_LOCKED;
	case -EAGAIN:
		return SR_ERR_CALLBACK_SHELVE;
	case -ENOMEM:
		return SR_ERR_NO_MEMORY;
	case -EIO:
	default:
		return SR_ERR_SYS;
	}
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
		return SR_ERR_OK;

	srepo_assert(err != SR_ERR_INVAL_ARG);
	if (err == SR_ERR_NO_MEMORY)
		srepo_abort();

	return err;
}

sr_error_t
srepo_discard_oper_changes(sr_session_ctx_t * session, const char * module)
{
	srepo_assert(session);
	srepo_assert(!module || module[0]);

	sr_error_t err;

	err = sr_discard_oper_changes(session, module, 0);
	if (err == SR_ERR_OK)
		return SR_ERR_OK;

	srepo_assert(err != SR_ERR_INVAL_ARG);
	if (err == SR_ERR_NO_MEMORY)
		srepo_abort();

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

	return err;
}

#if defined(CONFIG_SREPO_PRINT)

sr_error_t
srepo_open_stdio_print(struct ly_out ** printer, FILE * stdio)
{
	srepo_assert(printer);
	srepo_assert(stdio);

	LY_ERR ret;

	ret = ly_out_new_file(stdio, printer);
	srepo_assert(ret != LY_EINVAL);
	if (ret == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(ret);
}

#endif /* defined(CONFIG_SREPO_PRINT) */
