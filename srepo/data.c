#include "srutils/srepo/data.h"
#include "common.h"
#include <errno.h>

#warning TODO: use srplg_log_errinfo() to push errors to clients.

/******************************************************************************
 * Yang data node value manipulation.
 ******************************************************************************/

sr_error_t
srepo_dat_node_dflt_as_bool(const struct lyd_node * node, bool * value)
{
	srepo_assert(srepo_dat_value_type(srepo_dat_node_value(node)) ==
	             LY_TYPE_BOOL);

	const char * dflt = srepo_dat_node_dflt(node);

	if (dflt) {
		*value = (!strcmp(dflt, "true")) ? true : false;
		return SR_ERR_OK;
	}

	return SR_ERR_NOT_FOUND;
}

/******************************************************************************
 * Yang data node manipulation.
 ******************************************************************************/

sr_error_t
srepo_dat_change_bypath(sr_session_ctx_t * session,
                        const char *       path,
                        const char *       value,
                        const char *       origin,
                        uint32_t           flags)
{
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(value);
	srepo_assert(!(flags & ~(SR_EDIT_DEFAULT |
	                         SR_EDIT_NON_RECURSIVE |
	                         SR_EDIT_STRICT |
	                         SR_EDIT_ISOLATE)));

	sr_error_t ret;

	ret = sr_set_item_str(session, path, value, origin, flags);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;

	srepo_assert(ret != SR_ERR_INVAL_ARG);
	if (ret == SR_ERR_NO_MEMORY)
		srepo_abort();

	return ret;
}

sr_error_t
srepo_dat_vchangef_bypath(sr_session_ctx_t * session,
                          const char *       path,
                          const char *       origin,
                          uint32_t           flags,
                          const char *       format,
                          va_list            args)
{
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(!(flags & ~(SR_EDIT_DEFAULT |
	                         SR_EDIT_NON_RECURSIVE |
	                         SR_EDIT_STRICT |
	                         SR_EDIT_ISOLATE)));
	srepo_assert(format);
	srepo_assert(format[0]);

	char * val;
	int    ret;

	ret = srepo_vasprintf(&val, format, args);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = sr_set_item_str(session, path, val, origin, flags);
	srepo_free(val);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;

	srepo_assert(ret != SR_ERR_INVAL_ARG);
	if (ret == SR_ERR_NO_MEMORY)
		srepo_abort();

	return ret;
}

sr_error_t
srepo_dat_new_node(const struct ly_ctx * context,
                   struct lyd_node *     parent,
                   const char *          path,
                   const char *          value,
                   uint32_t              options,
                   struct lyd_node **    nevv)
{
	srepo_assert(context || parent);
	srepo_assert(srepo_xpath_validate(path) > 0);

	LY_ERR ret;

	ret = lyd_new_path(parent, context, path, value, options, nevv);
	if (ret == LY_SUCCESS)
		return SR_ERR_OK;

	srepo_assert(ret != LY_EINVAL);
	srepo_assert(ret != LY_EVALID);
	srepo_assert(ret != LY_EEXIST);
	if (ret == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_new_dflt_nodes(struct lyd_node *  tree,
                         uint32_t           options,
                         struct lyd_node ** diff)
{
	srepo_assert(tree);
	srepo_assert(!(options & ~SREPO_DAT_IMPLICIT_OPTS));

	LY_ERR ret;

	ret = lyd_new_implicit_tree(tree, options, diff);
	srepo_assert(ret != LY_EINVAL);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_create_list_keyent(const struct ly_ctx * context,
                             struct lyd_node *     parent,
                             const char *          path,
                             const char *          key,
                             const char *          value,
                             struct lyd_node **    entry)
{
	srepo_assert(context || parent);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(parent || (path[0] == '/'));
	srepo_assert(key);
	srepo_assert(key[0]);
	srepo_assert(value);
	srepo_assert(value[0]);

	char * kpath;
	int    ret;

	ret = srepo_asprintf(&kpath, "%s[%s='%s']", path, key, value);
	srepo_assert(ret);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = srepo_dat_new_node(context, parent, kpath, value, 0, entry);

	srepo_free(kpath);

	return ret;
}

sr_error_t
srepo_dat_vcreatef_leaf(struct lyd_node *  parent,
                        const char *       path,
                        struct lyd_node ** leaf,
                        const char *       format,
                        va_list            args)
{
	srepo_assert(parent);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(format);
	srepo_assert(format[0]);

	char * val;
	int    ret;

	ret = srepo_vasprintf(&val, format, args);
	srepo_assert(ret);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = srepo_dat_new_node(NULL, parent, path, val, 0, leaf);

	srepo_free(val);

	return ret;
}

/******************************************************************************
 * Searching YANG data nodes / trees.
 ******************************************************************************/

sr_error_t
srepo_dat_find_node(const struct lyd_node * tree,
                    const char *            path,
                    struct lyd_node **      node)
{
	srepo_assert(tree);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(node);

	LY_ERR ret;

	ret = lyd_find_path(tree, path, 0, node);
	srepo_assert(ret != LY_EINVAL);
	if (ret == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_vfindf_node(const struct lyd_node * tree,
                      struct lyd_node **      node,
                      const char *            format,
                      va_list                 args)
{
	srepo_assert(tree);
	srepo_assert(node);
	srepo_assert(format);
	srepo_assert(format[0]);

	int    ret;
	char * path;

	ret = srepo_vasprintf(&path, format, args);
	srepo_assert(ret);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = srepo_dat_find_node(tree, path, node);

	srepo_free(path);

	return ret;
}

/******************************************************************************
 * Loading YANG data nodes / trees.
 ******************************************************************************/

sr_error_t
srepo_dat_load_data(sr_session_ctx_t * session,
                    const char *       path,
                    unsigned int       depth,
                    sr_get_oper_flag_t flags,
                    sr_data_t **       data)
{
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_dat_assert_flags(flags);
	srepo_assert(data);

	sr_error_t err;

	err = sr_get_data(session, path, depth, 0, flags, data);
	if (err != SR_ERR_OK) {
		if (err == SR_ERR_NOT_FOUND)
			/* Path is invalid: no nodes will ever match it. */
			err = SR_ERR_INVAL_ARG;
		else if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		return err;
	}

	if (!*data)
		/* Valid path but no corresponding data subtree(s) found. */
		return SR_ERR_NOT_FOUND;

	srepo_assert((*data)->tree);

	return SR_ERR_OK;
}

sr_error_t
srepo_dat_load_node(sr_session_ctx_t * session,
                    const char *       path,
                    sr_data_t **       data)
{
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(data);

	sr_error_t err;

	err = sr_get_node(session, path, 0, data);
	if (err != SR_ERR_OK) {
		if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		return err;
	}

	srepo_assert(*data);
	srepo_assert((*data)->tree);
	srepo_assert(LYD_NODE_IS_ALONE((*data)->tree));

	return SR_ERR_OK;
}

/******************************************************************************
 * Debugging / printing YANG data nodes / trees.
 ******************************************************************************/

#if defined(CONFIG_SREPO_PRINT)

#define srepo_dat_isprint_format_valid(_fmt) \
	(((_fmt) == LYD_XML) || ((_fmt) == LYD_JSON) || ((_fmt) == LYD_LYB))

sr_error_t
srepo_dat_print_stdio_data(const sr_data_t * data,
                           LYD_FORMAT        format,
                           FILE *            stdio)
{
	srepo_assert(data);
	srepo_assert(srepo_dat_isprint_format_valid(format));
	srepo_assert(stdio);

	LY_ERR ret;

	ret = lyd_print_file(stdio,
	                     data->tree,
	                     format,
	                     LYD_PRINT_EMPTY_LEAF_LIST |
	                     LYD_PRINT_WD_IMPL_TAG |
	                     LYD_PRINT_SIBLINGS);
	srepo_assert(ret != LY_EINVAL);
	if (ret == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_print_data(const sr_data_t * data,
                     LYD_FORMAT        format,
                     struct ly_out *   printer)
{
	srepo_assert(data);
	srepo_assert(srepo_dat_isprint_format_valid(format));
	srepo_assert(printer);

	LY_ERR ret;

	ret = lyd_print_all(printer,
	                    data->tree,
	                    format,
	                    LYD_PRINT_EMPTY_LEAF_LIST | LYD_PRINT_WD_IMPL_TAG);
	srepo_assert(ret != LY_EINVAL);
	if (ret == LY_EMEM)
		srepo_abort();

	return srepo_ly_error(ret);
}

#endif /* defined(CONFIG_SREPO_PRINT) */
