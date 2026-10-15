#include "log.h"
#include "srutils/srepo/data.h"
#include "srutils/srepo/schema.h"

#if defined(CONFIG_SRPLUG_DAEMON)
#include "srutils/srplug/daemon.h"
#elif defined(CONFIG_SRPLUG_THREAD)
#include "srutils/srplug/thread.h"
#else
#error Invalid build configuration: no implementation found !
#endif

/******************************************************************************
 * Sysrepo data changes handling.
 ******************************************************************************/

#if defined(CONFIG_SRPLUG_DEBUG)

static
const char *
srplug_change_oper_str(sr_change_oper_t oper)
{
	switch (oper) {
	case SR_OP_CREATED:
		return "created";
	case SR_OP_MODIFIED:
		return "modified";
	case SR_OP_DELETED:
		return "deleted";
	case SR_OP_MOVED:
		return "moved";
	default:
		break;
	}

	return "unknown";
}

sr_error_t
srplug_change_debug(const sr_session_ctx_t * session,
                    const struct lyd_node *  node,
                    sr_change_oper_t         oper,
                    const char *             old,
                    void *                   data)
{
	srplug_assert(session);
	srplug_assert(node);

	srplug_sess_node_debug(session,
	                       node,
	                       "%s change event: %s%s%s --> '%s' [data:%p]",
	                       srplug_change_oper_str(oper),
	                       old ? "'" : "",
	                       old ? old : "none",
	                       old ? "'" : "",
	                       srepo_dat_node_as_str(node),
	                       data);
	return SR_ERR_OK;
}

#endif /* defined(CONFIG_SRPLUG_DEBUG) */

sr_error_t
srplug_handle_changes(sr_session_ctx_t *        session,
                      const char *              xpath,
                      srplug_handle_change_fn * handle,
                      void *                    data)
{
	srplug_assert(session);
	srplug_assert(xpath);
	srplug_assert(handle);

	sr_change_iter_t * iter;
	sr_error_t         ret;

	if (srepo_xpath_validate(xpath) < 0) {
		srplug_assert(0);
		return SR_ERR_INTERNAL;
	}

	ret = sr_get_changes_iter(session, xpath, &iter);
	srplug_assert(ret != SR_ERR_INVAL_ARG);
	if (ret == SR_ERR_NO_MEMORY)
		srepo_abort();
	else if (ret != SR_ERR_OK)
		return ret;

	do {
		sr_change_oper_t        oper;
		const struct lyd_node * node;
		const char *            old;

		ret = sr_get_change_tree_next(session,
		                              iter,
		                              &oper,
		                              &node,
		                              &old,
		                              NULL,
		                              NULL);
		srplug_assert(ret != SR_ERR_INVAL_ARG);
		if (ret == SR_ERR_NO_MEMORY)
			srepo_abort();
		if (ret != SR_ERR_OK)
			break;

		ret = handle(session, node, oper, old, data);
	} while (ret == SR_ERR_OK);

	sr_free_change_iter(iter);

	return (ret == SR_ERR_NOT_FOUND) ? SR_ERR_OK : ret;
}

/*
 * Sysrepo pluging data change dispatcher.
 */
struct srplug_change_dispatch {
	unsigned int                       nr;
	const struct srplug_change_hndlr * hndlrs;
	void *                             data;
};

#define srplug_assert_change_dispatch(_dispatch) \
	srplug_assert(_dispatch); \
	srplug_assert((_dispatch)->nr); \
	srplug_assert((_dispatch)->hndlrs)

#define srplug_assert_change_hndlr(_hndlr) \
	srplug_assert(_hndlr); \
	srplug_assert(srepo_xpath_validate_node((_hndlr)->name)); \
	srplug_assert((_hndlr)->handle)

static sr_error_t
srplug_dispatch_child_change(sr_session_ctx_t *      session,
                             const struct lyd_node * node,
                             sr_change_oper_t        oper,
                             const char *            old,
                             void *                  dispatch)
{
	srplug_assert_change_dispatch((const struct srplug_change_dispatch *)
	                              dispatch);

	const char *                          name;
	unsigned int                          h;
	const struct srplug_change_dispatch * disp = dispatch;

	name = srepo_dat_node_name(node);
	if (!name) {
		srplug_sess_node_warn(session, node, "invalid node name");
		return SR_ERR_INVAL_ARG;
	}

	for (h = 0; h < disp->nr; h++) {
		const struct srplug_change_hndlr * hndlr = &disp->hndlrs[h];

		srplug_assert_change_hndlr(hndlr);

		if ((!hndlr->feature || hndlr->feature->on) &&
		    !strcmp(name, hndlr->name)) {
			srplug_sess_node_debug(session,
			                       node,
			                       "handling '%s' change operation",
			                       srplug_change_oper_str(oper));
			return hndlr->handle(session,
			                     node,
			                     oper,
			                     old,
			                     disp->data);
		}
	}

	srplug_sess_node_debug(session,
	                       node,
	                       "no change handler found: ignoring");

	return SR_ERR_OK;
}

sr_error_t
srplug_process_child_changes(sr_session_ctx_t *                 session,
                             const char *                       xpath,
                             const struct srplug_change_hndlr * handlers,
                             unsigned int                       nr,
                             void *                             data)
{
	srplug_assert(session);
	srplug_assert(srepo_xpath_validate(xpath) > 0);
	srplug_assert(handlers);
	srplug_assert(nr);

	char *                              pth;
	int                                 ret;
	const struct srplug_change_dispatch disp = {
		.nr     = nr,
		.hndlrs = handlers,
		.data   = data
	};

	ret = srepo_xpath_createf(&pth, "%s/*", xpath);
	srplug_assert(ret);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = srplug_handle_changes(session,
	                            pth,
	                            srplug_dispatch_child_change,
	                            (void *)&disp);
	srepo_free(pth);

	return ret;
}

sr_error_t
srplug_probe_feature(const struct ly_ctx * context,
                     const char *          module,
                     struct srplug_feat *  feature)
{
	srplug_assert(context);
	srplug_assert(module);
	srplug_assert(module[0]);
	srplug_assert(feature);
	srplug_assert(feature->name);
	srplug_assert(feature->name[0]);

	const struct lys_module * mod;
	LY_ERR                    ret;

	mod = srepo_sch_find_module(context, module);
	if (!mod) {
		srplug_warn("'%s' module: not found", mod->name);
		return SR_ERR_NOT_FOUND;
	}

	ret = lys_feature_value(mod, feature->name);
	switch (ret) {
	case LY_SUCCESS:
		feature->on = true;
		return SR_ERR_OK;

	case LY_ENOT:
		feature->on = false;
		return SR_ERR_OK;

	case LY_ENOTFOUND:
		break;

	default:
		srplug_assert(0);
	}

	srplug_warn("'%s' module: '%s' feature: not found",
	            mod->name,
	            feature->name);

	return SR_ERR_NOT_FOUND;
}

const struct lys_module *
srplug_find_module(const struct ly_ctx * context, const char * module)
{
	srplug_assert(context);
	srplug_assert(module);
	srplug_assert(module[0]);

	const struct lys_module * mod;

	mod = srepo_sch_find_module(context, module);
	if (mod)
		return mod;

	srplug_warn("'%s' module: not found", module);

	return NULL;
}
