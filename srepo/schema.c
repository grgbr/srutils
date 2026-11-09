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

	path = srepo_xpath_alloc();
	path = lysc_path(node, LYSC_PATH_DATA, path, SREPO_XPATH_SIZE);
	if (path) {
		srepo_assert(srepo_xpath_validate(path) > 0);
		return path;
	}

	/* No more memory or SREPO_XPATH_SIZE not large enought ! */
	srepo_assert(0);
	srepo_abort();
}

sr_error_t
srepo_sch_validate_value(const struct lysc_node *  node,
                         const char *              value,
                         size_t                    length,
                         const struct lysc_type ** type)
{
	srepo_assert(node);
	srepo_assert(value);
	srepo_assert(length <= UINT32_MAX);

	LY_ERR err;

	err = lyd_value_validate(node, value, length, NULL, type, NULL);
	if (err == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(err != LY_EINCOMPLETE);

	return srepo_ly_error(err);
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

/******************************************************************************
 * Yang schema path iteration (so-called atoms).
 ******************************************************************************/

#define SREPO_PATH_INIT_NR \
	(4U)

static int
srepo_sch_push_path_atom(struct srepo_sch_path *  path,
                         const struct lysc_node * atom)
{
	srepo_sch_assert_path(path);
	srepo_assert(atom);

	if (path->cnt == path->nr) {
		unsigned int nr = 2 * path->nr;

		if (nr >= SREPO_PATH_ATOM_MAX)
			return -ENOBUFS;

		path->nr = nr;
		path->atoms = srepo_realloc(path->atoms,
		                            nr * sizeof(path->atoms[0]));
		srepo_assert(path->atoms);
	}

	path->atoms[path->cnt++] = atom;

	return 0;
}

static void
srepo_sch_invert_path(struct srepo_sch_path * path)
{
	srepo_sch_assert_path(path);

	unsigned int cnt = path->cnt / 2;
	unsigned int c;

	for (c = 0; c < cnt; c++) {
		unsigned int             end = path->cnt - c - 1;
		const struct lysc_node * tmp = path->atoms[c];

		path->atoms[c] = path->atoms[end];
		path->atoms[end] = tmp;
	}
}

char *
srepo_sch_path_string(const struct srepo_sch_path * path)
{
	unsigned int              cnt;
	const struct lysc_node *  scn;
	const struct lys_module * mod = NULL;
	char *                    pth;
	char *                    str;
	size_t                    sz = SREPO_XPATH_SIZE;

	pth = srepo_malloc(SREPO_XPATH_SIZE);
	srepo_assert(pth);

	str = pth;
	srepo_sch_foreach_path_atom(path, cnt, scn) {
		const char * nname = srepo_sch_node_name(scn);
		size_t       len;

		len = strnlen(nname, SREPO_XPATH_SIZE);
		srepo_assert(len <= SREPO_XPATH_SIZE);
		if (!len)
			goto free;

		if (mod != scn->module) {
			const char * mname;
			size_t       mlen;

			mod = scn->module;

			mname = mod->name;
			mlen = strnlen(mname, SREPO_XPATH_SIZE);
			srepo_assert(mlen <= SREPO_XPATH_SIZE);
			if (!mlen || ((1 + mlen + 1 + len) >= sz))
				goto free;

			str[0] = '/';
			memcpy(&str[1], mname, mlen);
			str[1 + mlen] = ':';
			memcpy(&str[1 + mlen + 1], nname, len);

			len = 1 + mlen + 1 + len;
		}
		else {
			if ((1 + len) >= sz)
				goto free;

			str[0] = '/';
			memcpy(&str[1], nname, len);

			len = 1 + len;
		}

		str += len;
		sz -= len;
	}

	*str = '\0';

	return pth;

free:
	srepo_free(pth);

	return NULL;
}

static
void
srepo_sch_init_path(struct srepo_sch_path * path)
{
	srepo_assert(path);

	path->cnt = 0;
	path->nr = SREPO_PATH_INIT_NR;
	path->atoms = srepo_malloc(SREPO_PATH_INIT_NR * sizeof(path->atoms[0]));
	srepo_assert(path->atoms);
}

void
srepo_sch_init_path_from_node(struct srepo_sch_path *  path,
                              const struct lysc_node * node)
{
	srepo_assert(path);
	srepo_assert(node);

	srepo_sch_init_path(path);

	do {
		if (!(srepo_sch_node_type(node) &
		      (LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)))
			/* Skip schema only nodes. */
			srepo_sch_push_path_atom(path, node);
		node = srepo_sch_node_parent(node);
	} while (node);

	srepo_sch_invert_path(path);
}

/******************************************************************************
 * Logging YANG schema nodes.
 ******************************************************************************/

#if defined(CONFIG_SREPO_LOG)

void
srepo_sch_vlog_node(const struct lysc_node * __restrict node,
                    enum elog_severity                  severity,
                    const char * __restrict             prefix,
                    const char * __restrict             format,
                    va_list                             args)
{
	srepo_assert(srepo_logger);
	srepo_assert(node);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * path;

	path = srepo_sch_node_path(node);
	srepo_xpath_vlog(path, severity, prefix, format, args);
	srepo_free(path);
}

void
srepo_sch_vlog_sess_node(const sr_session_ctx_t * __restrict session,
                         const struct lysc_node * __restrict node,
                         enum elog_severity                  severity,
                         const char * __restrict             prefix,
                         const char * __restrict             format,
                         va_list                             args)
{
	srepo_assert(srepo_logger);
	srepo_assert(session);
	srepo_assert(node);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * path;

	path = srepo_sch_node_path(node);
	srepo_xpath_vlog_sess(session, path, severity, prefix, format, args);
	srepo_free(path);
}

#endif /* defined(CONFIG_SREPO_LOG) */
