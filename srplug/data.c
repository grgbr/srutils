#include "srutils/srplug/data.h"
#include "common.h"

#if defined(CONFIG_SRPLUG_DAEMON)
#include "srutils/srplug/daemon.h"
#elif defined(CONFIG_SRPLUG_THREAD)
#include "srutils/srplug/thread.h"
#else
#error Invalid build configuration: no implementation found !
#endif

/******************************************************************************
 * YANG xpath manipulation.
 ******************************************************************************/

char *
srplug_dat_path(const struct lyd_node * node)
{
	srplug_assert(node);

	char * path;

	path = srepo_dat_path(node);
	if (!path)
		srplug_abort();

	return path;
}

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

sr_error_t
srplug_dat_create_container(const struct ly_ctx * context,
                            struct lyd_node *     parent,
                            const char *          path,
                            struct lyd_node **    container)
{
	srplug_assert(context || parent);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(parent || (path[0] == '/'));

	sr_error_t ret;

	ret = srepo_dat_create_container(context, parent, path, container);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_pnode_notice(parent,
	                    path,
	                    "cannot create container: %s",
	                    sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_create_list_ent(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    entry)
{
	srplug_assert(context || parent);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(parent || (path[0] == '/'));

	sr_error_t ret;

	ret = srepo_dat_create_list_ent(context, parent, path, entry);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_pnode_notice(parent,
	                    path,
	                    "cannot create list entry: %s",
	                    sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_create_list_keyent(const struct ly_ctx * context,
                              struct lyd_node *     parent,
                              const char *          path,
                              const char *          key,
                              const char *          value,
                              struct lyd_node **    entry)
{
	srplug_assert(context || parent);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(parent || (path[0] == '/'));
	srplug_assert(key);
	srplug_assert(key[0]);
	srplug_assert(value);
	srplug_assert(value[0]);

	sr_error_t ret;

	ret = srepo_dat_create_list_keyent(context,
	                                   parent,
	                                   path,
	                                   key,
	                                   value,
	                                   entry);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_pnode_notice(parent,
	                    path,
	                    "cannot create keyed list entry: %s",
	                    sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_create_leaf(struct lyd_node *  parent,
                       const char *       path,
                       const char *       value,
                       struct lyd_node ** leaf)
{
	srplug_assert(parent);
	srplug_assert(path);
	srplug_assert(path[0]);

	sr_error_t ret;

	ret = srepo_dat_create_leaf(parent, path, value, leaf);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_pnode_notice(parent,
	                    path,
	                    "cannot create leaf: %s",
	                    sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_create_leaf_vprintf(struct lyd_node *  parent,
                               const char *       path,
                               struct lyd_node ** leaf,
                               const char *       format,
                               va_list            args)
{
	srplug_assert(parent);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(format);

	sr_error_t ret;

	ret = srepo_dat_create_leaf_vprintf(parent, path, leaf, format, args);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_pnode_notice(parent,
	                    path,
	                    "cannot create leaf: %s",
	                    sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_create_leaf_printf(struct lyd_node *  parent,
                              const char *       path,
                              struct lyd_node ** leaf,
                              const char *       format,
                              ...)
{
	srplug_assert(parent);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(format);

	va_list args;
	int     ret;

	va_start(args, format);
	ret = srplug_dat_create_leaf_vprintf(parent,
	                                     path,
	                                     leaf,
	                                     format,
	                                     args);
	va_end(args);

	return ret;
}

sr_error_t
srplug_dat_populate_defaults(struct lyd_node *  tree,
                             uint32_t           options,
                             struct lyd_node ** diff)
{
	srplug_assert(tree);
	srplug_assert(!(options & ~SREPO_DAT_IMPLICIT_OPTS));

	sr_error_t ret;

	ret = srepo_dat_new_implicit(tree, options, diff);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_node_notice(tree,
	                   "cannot populate with default nodes: %s",
	                   sr_strerror(ret));

	return ret;
}

sr_error_t
srplug_dat_merge_batch(sr_session_ctx_t *      session,
                       const struct lyd_node * trees)
{
	srplug_assert(session);
	srplug_assert(trees);

	sr_error_t ret;

	ret = srepo_dat_merge_batch(session, trees);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	srplug_notice("cannot prepare changes batch: %s", sr_strerror(ret));

	return ret;
}

/******************************************************************************
 * Searching YANG data nodes / trees.
 ******************************************************************************/

sr_error_t
srplug_dat_find_path(const struct lyd_node * tree,
                     const char *            path,
                     struct lyd_node **      node)
{
	srplug_assert(tree);
	srplug_assert(path);
	srplug_assert(path[0]);
	srplug_assert(node);

	sr_error_t ret;

	ret = srepo_dat_find_path(tree, path, node);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

sr_error_t
srplug_dat_find_vpathf(const struct lyd_node * tree,
                       struct lyd_node **      node,
                       const char *            format,
                       va_list                 args)
{
	sr_error_t ret;

	ret = srepo_dat_find_vpathf(tree, node, format, args);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

/******************************************************************************
 * Loading YANG data nodes / trees.
 ******************************************************************************/

sr_error_t
srplug_dat_load_data(sr_session_ctx_t * session,
                     const char *       xpath,
                     unsigned int       depth,
                     sr_get_oper_flag_t flags,
                     sr_data_t **       data)
{
	srplug_assert(session);
	srplug_assert(xpath);
	srplug_assert(xpath[0]);
	srepo_dat_assert_flags(flags);
	srplug_assert(data);

	sr_error_t ret;

	ret = srepo_dat_load_data(session, xpath, depth, flags, data);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

sr_error_t
srplug_dat_load_node(sr_session_ctx_t * session,
                     const char *       xpath,
                     sr_data_t **       data)
{
	srplug_assert(session);
	srplug_assert(xpath);
	srplug_assert(xpath[0]);
	srplug_assert(data);

	sr_error_t ret;

	ret = srepo_dat_load_node(session, xpath, data);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

/******************************************************************************
 * Debugging / printing YANG data nodes / trees.
 ******************************************************************************/

#if defined(CONFIG_SRPLUG_PRINT)

sr_error_t
srplug_dat_print_stdio_data(const sr_data_t * data,
                            LYD_FORMAT        format,
                            FILE *            stdio)
{
	srplug_assert(data);
	srplug_assert(srepo_dat_isprint_format_valid(format));
	srplug_assert(stdio);

	sr_error_t ret;

	ret = srepo_dat_print_stdio_data(data, format, stdio);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

sr_error_t
srplug_dat_print_data(const sr_data_t * data,
                      LYD_FORMAT        format,
                      struct ly_out *   printer)
{
	srplug_assert(data);
	srplug_assert(srepo_dat_isprint_format_valid(format));
	srplug_assert(printer);

	sr_error_t ret;

	ret = srepo_dat_print_data(data, format, printer);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

sr_error_t
srplug_open_stdio_print(struct ly_out ** printer, FILE * stdio)
{
	srplug_assert(printer);
	srplug_assert(stdio);

	sr_error_t ret;

	ret = srepo_open_stdio_print(printer, stdio);
	if (ret == SR_ERR_OK)
		return SR_ERR_OK;
	else if (ret == SR_ERR_NO_MEMORY)
		srplug_abort();

	return ret;
}

#endif /* defined(CONFIG_SRPLUG_PRINT) */
