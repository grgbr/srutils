#include "srutils/srepo/data.h"
#include <errno.h>

#warning TODO: use srplg_log_errinfo() to push errors to clients.

static sr_error_t
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

static inline LY_ERR
srepo_dat_new_path(const struct ly_ctx * context,
                   struct lyd_node *     parent,
                   const char *          path,
                   const char *          value,
                   uint32_t              options,
                   struct lyd_node **    nevv)
{
	srepo_assert(context || parent);
	srepo_assert(path);
	srepo_assert(path[0]);

	LY_ERR ret;

	ret = lyd_new_path(parent, context, path, value, options, nevv);
	srepo_assert(ret != LY_EINVAL);
	srepo_assert(ret != LY_EVALID);

	return ret;
}

sr_error_t
srepo_dat_create_container(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    container)
{
	srepo_assert(context || parent);
	srepo_assert(path);
	srepo_assert(path[0]);
	srepo_assert(parent || (path[0] == '/'));

	LY_ERR ret;

	ret = srepo_dat_new_path(context, parent, path, NULL, 0, container);
	srepo_assert(ret != LY_EEXIST);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_create_list_ent(const struct ly_ctx * context,
                          struct lyd_node *     parent,
                          const char *          path,
                          struct lyd_node **    entry)
{
	srepo_assert(context || parent);
	srepo_assert(path);
	srepo_assert(path[0]);
	srepo_assert(parent || (path[0] == '/'));

	LY_ERR ret;

	ret = srepo_dat_new_path(context, parent, path, NULL, 0, entry);
	srepo_assert(ret != LY_EEXIST);

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
	srepo_assert(path);
	srepo_assert(path[0]);
	srepo_assert(parent || (path[0] == '/'));
	srepo_assert(key);
	srepo_assert(key[0]);
	srepo_assert(value);
	srepo_assert(value[0]);

	int    ret;
	char * kpath;

	ret = asprintf(&kpath, "%s[%s='%s']", path, key, value);
	srepo_assert(ret);
	if (ret < 0) {
		if (errno == ENOMEM)
			return SR_ERR_NO_MEMORY;
		return SR_ERR_LY;
	}

	ret = srepo_dat_new_path(context, parent, kpath, value, 0, entry);
	srepo_assert(ret != LY_EEXIST);

	free(kpath);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_create_leaf(struct lyd_node *  parent,
                      const char *       path,
                      const char *       value,
                      struct lyd_node ** leaf)
{
	srepo_assert(parent);
	srepo_assert(path);
	srepo_assert(path[0]);

	LY_ERR ret;

	ret = srepo_dat_new_path(NULL, parent, path, value, 0, leaf);
	srepo_assert(ret != LY_EEXIST);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_create_leaf_vprintf(struct lyd_node *  parent,
                              const char *       path,
                              struct lyd_node ** leaf,
                              const char *       format,
                              va_list            args)
{
	srepo_assert(parent);
	srepo_assert(path);
	srepo_assert(path[0]);
	srepo_assert(format);

	int    ret;
	char * val;

	ret = vasprintf(&val, format, args);
	srepo_assert(ret);
	if (ret < 0) {
		if (errno == ENOMEM)
			return SR_ERR_NO_MEMORY;
		return SR_ERR_LY;
	}

	ret = srepo_dat_new_path(NULL, parent, path, val, 0, leaf);
	srepo_assert(ret != LY_EEXIST);

	free(val);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_dat_create_leaf_printf(struct lyd_node *  parent,
                             const char *       path,
                             struct lyd_node ** leaf,
                             const char *       format,
                             ...)
{
	srepo_assert(parent);
	srepo_assert(path);
	srepo_assert(path[0]);
	srepo_assert(format);

	va_list args;
	int     ret;

	va_start(args, format);
	ret = srepo_dat_create_leaf_vprintf(parent,
	                                    path,
	                                    leaf,
	                                    format,
	                                    args);
	va_end(args);

	return ret;
}

sr_error_t
srepo_dat_new_implicit(struct lyd_node *  tree,
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

/******************************************************************************
 * Searching for / loading YANG data nodes / trees.
 ******************************************************************************/

sr_error_t
srepo_dat_load(sr_session_ctx_t * session,
               const char *       xpath,
               unsigned int       depth,
               sr_get_oper_flag_t flags,
               sr_data_t **       data)
{
	srepo_assert(session);
	srepo_assert(xpath);
	srepo_assert(xpath[0]);
	srepo_dat_assert_flags(flags);
	srepo_assert(data);

	sr_error_t err;

	err = sr_get_data(session, xpath, depth, 0, flags, data);
	if (err != SR_ERR_OK) {
		if (err == SR_ERR_NOT_FOUND)
			/* Path is invalid: no nodes will ever match it. */
			err = SR_ERR_INVAL_ARG;
		return err;
	}

	if (!*data)
		/* Valid path but no corresponding data subtree(s) found. */
		return SR_ERR_NOT_FOUND;

	srepo_assert((*data)->tree);

	return SR_ERR_OK;
}

/******************************************************************************
 * Debugging / printing YANG data nodes / trees.
 ******************************************************************************/

#if defined(CONFIG_SREPO_PRINT)

sr_error_t
srepo_dat_print_data(const sr_data_t * data,
                     LYD_FORMAT        format,
                     struct ly_out *   printer)
{
	srepo_assert(data);
	srepo_assert(printer);
	srepo_assert(srepo_dat_isprint_format_valid(format));

	LY_ERR ret;

	ret = lyd_print_all(printer,
	                    data->tree,
	                    format,
	                    LYD_PRINT_EMPTY_LEAF_LIST | LYD_PRINT_WD_IMPL_TAG);
	srepo_assert(ret != LY_EINVAL);

	return srepo_ly_error(ret);
}

sr_error_t
srepo_open_stdio_print(struct ly_out ** printer, FILE * stdio)
{
	srepo_assert(printer);
	srepo_assert(stdio);

	LY_ERR ret;

	ret = ly_out_new_file(stdio, printer);
	srepo_assert(ret != LY_EINVAL);

	return srepo_ly_error(ret);
}

void
srepo_close_stdio_print(struct ly_out * printer)
{
	srepo_assert(printer);

	ly_out_free(printer, NULL, 0);
}

#endif /* defined(CONFIG_SREPO_PRINT) */
