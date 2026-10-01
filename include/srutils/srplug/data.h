#ifndef _SRPLUG_DATA_H
#define _SRPLUG_DATA_H

#include <srutils/srplug/common.h>
#include <srutils/srepo/data.h>

/******************************************************************************
 * YANG xpath manipulation.
 ******************************************************************************/

extern char *
srplug_dat_path(const struct lyd_node * node)
	__srplug_export;

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

extern sr_error_t
srplug_dat_create_container(const struct ly_ctx * context,
                            struct lyd_node *     parent,
                            const char *          path,
                            struct lyd_node **    container)
	__srplug_export;

extern sr_error_t
srplug_dat_create_list_ent(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    entry)
	__srplug_export;

extern sr_error_t
srplug_dat_create_list_keyent(const struct ly_ctx * context,
                              struct lyd_node *     parent,
                              const char *          path,
                              const char *          key,
                              const char *          value,
                              struct lyd_node **    entry)
	__srplug_export;

extern sr_error_t
srplug_dat_create_leaf(struct lyd_node *  parent,
                       const char *       path,
                       const char *       value,
                       struct lyd_node ** leaf)
	__srplug_export;

extern sr_error_t
srplug_dat_create_leaf(struct lyd_node *  parent,
                       const char *       path,
                       const char *       value,
                       struct lyd_node ** leaf)
	__srepo_export;

extern sr_error_t
srplug_dat_create_leaf_vprintf(struct lyd_node *  parent,
                               const char *       path,
                               struct lyd_node ** leaf,
                               const char *       format,
                               va_list            args)
	__srepo_export;

extern sr_error_t
srplug_dat_create_leaf_printf(struct lyd_node *  parent,
                              const char *       path,
                              struct lyd_node ** leaf,
                              const char *       format,
                              ...)
	__srepo_export;

extern sr_error_t
srplug_dat_populate_defaults(struct lyd_node *  tree,
                             uint32_t           options,
                             struct lyd_node ** diff)
	__srplug_export;

static inline void
srplug_dat_free_tree(struct lyd_node * tree)
{
	srepo_dat_free_tree(tree);
}

/******************************************************************************
 * Searching YANG data nodes / trees.
 ******************************************************************************/

extern sr_error_t
srplug_dat_find_path(const struct lyd_node * tree,
                     const char *            path,
                     struct lyd_node **      node)
	__srplug_export;

extern sr_error_t
srplug_dat_find_vpathf(const struct lyd_node * tree,
                       struct lyd_node **      node,
                       const char *            format,
                       va_list                 args)
	__srplug_export;

static inline sr_error_t
srplug_dat_find_pathf(const struct lyd_node * tree,
                      struct lyd_node **      node,
                      const char *            format,
                      ...)
{
	srplug_assert(tree);
	srplug_assert(node);
	srplug_assert(format);
	srplug_assert(format[0]);

	va_list    args;
	sr_error_t ret;

	va_start(args, format);
	ret = srplug_dat_find_vpathf(tree, node, format, args);
	va_end(args);

	return ret;
}

/******************************************************************************
 * Loading YANG data nodes / trees.
 ******************************************************************************/

extern sr_error_t
srplug_dat_load_data(sr_session_ctx_t * session,
                     const char *       xpath,
                     unsigned int       depth,
                     sr_get_oper_flag_t flags,
                     sr_data_t **       data)
	__srepo_export;

extern sr_error_t
srplug_dat_load_node(sr_session_ctx_t * session,
                     const char *       xpath,
                     sr_data_t **       data)
	__srplug_export;

static inline void
srplug_dat_release_data(sr_data_t * data)
{
	srplug_assert(data);

	srepo_dat_release_data(data);
}

extern sr_error_t
srplug_dat_merge_batch(sr_session_ctx_t *      session,
                       const struct lyd_node * trees)
	__srplug_export;

static inline sr_error_t
srplug_dat_merge_data_batch(sr_session_ctx_t * session, const sr_data_t * data)
{
	srplug_assert(session);
	srplug_assert(data);
	srplug_assert(data->tree);

	return srplug_dat_merge_batch(session, data->tree);
}

/******************************************************************************
 * Debugging / printing YANG data nodes / trees.
 ******************************************************************************/

#if defined(CONFIG_SRPLUG_PRINT)

extern sr_error_t
srplug_dat_print_stdio_data(const sr_data_t * data,
                            LYD_FORMAT        format,
                            FILE *            stdio)
	__srepo_export;

extern sr_error_t
srplug_dat_print_data(const sr_data_t * data,
                      LYD_FORMAT        format,
                      struct ly_out *   printer)
	__srplug_export;

extern sr_error_t
srplug_open_stdio_print(struct ly_out ** printer, FILE * stdio)
	__srplug_export;

static inline void
srplug_close_stdio_print(struct ly_out * printer)
{
	srplug_assert(printer);

	srepo_close_stdio_print(printer);
}

#endif /* defined(CONFIG_SRPLUG_PRINT) */

#endif /* _SRPLUG_DATA_H */
