#include "common.h"
#include "srutils/srepo/schema.h"
#include "srutils/srepo/data.h"

/******************************************************************************
 * Libyang (compiled) schema node handling.
 ******************************************************************************/

char *
srepo_sch_node_path(const struct lysc_node * node)
{
	srepo_assert(node);

	char * path;
	char * pth;

	path = srepo_xpath_alloc();
	pth = lysc_path(node, LYSC_PATH_DATA, NULL, 0);
	if (pth) {
		srepo_assert(srepo_xpath_validate(path) > 0);
		return pth;
	}

	/* No more memory or SREPO_XPATH_SIZE not large enought ! */
	srepo_assert(0);
	srepo_abort();
}

/******************************************************************************
 * Searching / browsing YANG (compiled) schema nodes / trees.
 ******************************************************************************/

#warning Remove recursion
sr_error_t
srepo_sch_walk_tree(const struct lysc_node * tree,
                    srepo_sch_visit_fn *     visit,
                    void *                   data)
{
	srepo_assert(tree);
	srepo_assert(visit);

	int ret;

	ret = visit(tree, SREPO_PRE_WALK_EVT, data);
	srepo_assert(ret >= SREPO_SKIP_WALK_RET);
	if (ret == SREPO_CONT_WALK_RET) {
		const struct lysc_node * child;

		srepo_sch_foreach_child(tree, child) {
			ret = srepo_sch_walk_tree(child, visit, data);
			if (ret > SREPO_CONT_WALK_RET) {
				/*
				 * An error has been reported: interrupt
				 * traversal and return.
				 */
				return ret;
			}
		}

		/* Cannot skip node while travelling back up. */
		ret = visit(tree, SREPO_POST_WALK_EVT, data);
		srepo_assert(ret >= SREPO_CONT_WALK_RET);
	}

	return (ret <= SREPO_CONT_WALK_RET) ? SREPO_CONT_WALK_RET : ret;
}

#if defined(CONFIG_SREPO_PRINT)

sr_error_t
srepo_sch_print_node_yang(struct ly_out *          printer,
                          const struct lysc_node * tree,
                          bool                     nosub)
{
	srepo_assert(printer);
	srepo_assert(tree);

	LY_ERR err;

	/*
	 * Note: line width argument is ignored when outputting node YANG
	 * specification.
	 */
	err = lys_print_node(printer,
	                     tree,
	                     LYS_OUT_YANG_COMPILED,
	                     0,
	                     nosub ? LYS_PRINT_NO_SUBSTMT : 0);
	if (err == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(err != LY_EINVAL);
	if (err == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(err);
}

sr_error_t
srepo_sch_print_node_diag(struct ly_out *          printer,
                          const struct lysc_node * tree,
                          unsigned int             cols_nr)
{
	srepo_assert(printer);
	srepo_assert(tree);

	LY_ERR err;

	/*
	 * Note: the LYS_PRINT_NO_SUBSTMT option is ignored when outputting
	 * node YANG tree diagram.
	 */
	err = lys_print_node(printer, tree, LYS_OUT_TREE, cols_nr, 0);
	if (err == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(err != LY_EINVAL);
	if (err == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(err);
}

#endif /* defined(CONFIG_SREPO_PRINT) */

/******************************************************************************
 * Loading Yang data from (compiled) schema nodes.
 ******************************************************************************/

sr_error_t
srepo_sch_load_data(sr_session_ctx_t *       session,
                    const struct lysc_node * node,
                    unsigned int             depth,
                    sr_get_oper_flag_t       flags,
                    sr_data_t **             data)
{
	srepo_assert(session);
	srepo_assert(node);
	srepo_dat_assert_get_flags(flags);
	srepo_assert(data);

	char *     path;
	sr_error_t ret;

	path = srepo_sch_node_path(node);
	ret = srepo_dat_load_data(session, path, depth, flags, data);
	srepo_free(path);

	return ret;
}

sr_error_t
srepo_sch_load_node(sr_session_ctx_t *       session,
                    const struct lysc_node * node,
                    sr_data_t **             data)
{
	srepo_assert(session);
	srepo_assert(node);
	srepo_assert(data);

	char *     path;
	sr_error_t ret;

	path = srepo_sch_node_path(node);
	ret = srepo_dat_load_node(session, path, data);
	srepo_free(path);

	return ret;
}

sr_error_t
srepo_sch_load_subtree(sr_session_ctx_t *       session,
                       const struct lysc_node * node,
                       sr_data_t **             data)
{
	srepo_assert(session);
	srepo_assert(node);
	srepo_assert(data);

	char *     path;
	sr_error_t ret;

	path = srepo_sch_node_path(node);
	ret = srepo_dat_load_subtree(session, path, data);
	srepo_free(path);

	return ret;
}

/******************************************************************************
 * Libyang (compiled) schema extension handling.
 ******************************************************************************/

bool
srepo_sch_is_extension(const struct lysc_ext_instance * extension,
                       const char *                     identifier)
{
	srepo_assert(extension);
	srepo_assert(extension->def);
	srepo_assert(extension->def->module);
	srepo_assert(identifier);
	srepo_assert(identifier[0]);

	const struct lysc_ext *   def = extension->def;
	const struct lys_module * mod = def->module;

	return !strcmp(mod->name, "cly-extensions") &&
	       !strcmp(mod->ns, "urn:cly:yang:cly-extensions") &&
	       !(def->flags & LYS_STATUS_DEPRC) &&
	       !strcmp(def->name, identifier);
}

/******************************************************************************
 * Libyang modules / features handling.
 ******************************************************************************/

sr_error_t
srepo_sch_feature_status(const struct lys_module * module,
                         const char *              feature,
                         bool *                    enabled)
{
	srepo_assert(module);
	srepo_assert(feature);
	srepo_assert(feature[0]);
	srepo_assert(enabled);

	LY_ERR err;

	err = lys_feature_value(module, feature);
	switch (err) {
	case LY_SUCCESS:
		*enabled = true;
		break;

	case LY_ENOT:
		*enabled = false;
		break;

	case LY_ENOTFOUND:
		return SR_ERR_NOT_FOUND;

	default:
		break;
	}

	srepo_assert(0);

	return SR_ERR_INTERNAL;
}

#if defined(CONFIG_SREPO_PRINT)

sr_error_t
srepo_sch_print_module_yang(struct ly_out *           printer,
                            const struct lys_module * module,
                            bool                      nosub)
{
	srepo_assert(printer);
	srepo_assert(module);

	LY_ERR err;

	/*
	 * Note: line width argument is ignored when outputting module YANG
	 * specification.
	 */
	err = lys_print_module(printer,
	                       module,
	                       LYS_OUT_YANG_COMPILED,
	                       0,
	                       nosub ? LYS_PRINT_NO_SUBSTMT : 0);
	if (err == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(err != LY_EINVAL);
	if (err == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(err);
}

sr_error_t
srepo_sch_print_module_diag(struct ly_out *           printer,
                            const struct lys_module * module,
                            unsigned int              cols_nr)
{
	srepo_assert(printer);
	srepo_assert(module);

	LY_ERR err;

	/*
	 * Note: line width argument is ignored when outputting module YANG
	 * specification.
	 */
	err = lys_print_module(printer,
	                       module,
	                       LYS_OUT_TREE,
	                       cols_nr,
	                       0);
	if (err == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(err != LY_EINVAL);
	if (err == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(err);
}

#endif /* defined(CONFIG_SREPO_PRINT) */

const struct lys_module *
srepo_sch_next_module(const struct ly_ctx * context, unsigned int * index)
{
	srepo_assert(context);
	srepo_assert(index);

	const struct lys_module * mod;

	mod = ly_ctx_get_module_iter(context, index);
	while (mod) {
		if (mod->implemented &&      /* implemented, not just imported */
		    (mod->compiled->data ||  /* has schema nodes */
		     mod->compiled->exts) && /* has schema extension instances */
		    !sr_is_module_internal(mod)) {
			/*
			 * Return external implemented modules that hold
			 * top-level data node(s).
			 */
			break;
		}

		mod = ly_ctx_get_module_iter(context, index);
	}

	return mod;
}

sr_error_t
srepo_sch_walk_module(const struct lys_module * module,
                      srepo_sch_visit_fn *      visit,
                      void *                    data)
{
	srepo_assert(module);
	srepo_assert(module->compiled);
	srepo_assert(visit);

	const struct lysc_node * root;
	sr_error_t               ret = SR_ERR_OK;

	/*
	 * Iterate over schema nodes only, i.e., not actions / rpcs, neither
	 * notifications (which may be reached thanks to the parent container
	 * lysc_node).
	 */
	LY_LIST_FOR(module->compiled->data, root) {
		ret = srepo_sch_walk_tree(root, visit, data);
		if (ret != SR_ERR_OK)
			break;
	}

	return ret;
}

const struct lys_module *
srepo_sch_find_module(const struct ly_ctx * context, const char * module)
{
	srepo_assert(context);
	srepo_assert(module);
	srepo_assert(module[0]);

	const struct lys_module * mod;

	mod = ly_ctx_get_module_implemented(context, module);
	if (mod && !sr_is_module_internal(mod))
		return mod;

	return NULL;
}
