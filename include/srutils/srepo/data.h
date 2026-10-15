#ifndef _SREPO_DATA_H
#define _SREPO_DATA_H

#include <srutils/srepo/xpath.h>
#include <stdbool.h>
#include <stdarg.h>

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

static inline __srepo_nonull(1) __returns_nonull
const char *
srepo_dat_node_name(const struct lyd_node * node)
{
	srepo_assert(node);

	return LYD_NAME(node);
}

static inline uint16_t
srepo_dat_node_type(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);

	return node->schema->nodetype;
}

static inline __srepo_nonull(1) __returns_nonull
char *
srepo_dat_node_path(const struct lyd_node * node)
{
	srepo_assert(node);

	char *       path;
	const char * pth;

	path = srepo_xpath_alloc();

	pth = lyd_path(node, LYD_PATH_STD, path, SREPO_XPATH_SIZE);
	srepo_assert(pth);

	return path;
}

static inline __srepo_nonull(1) __returns_nonull
const struct lyd_value *
srepo_dat_node_value(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(srepo_dat_node_type(node) & LYD_NODE_TERM);

	return &((const struct lyd_node_term *)node)->value;
}

static inline const char *
srepo_dat_node_dflt(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(srepo_dat_node_type(node) == LYS_LEAF);

	const struct lysc_node_leaf * leaf = (const struct lysc_node_leaf *)
	                                     node->schema;
	return leaf->dflt.str;
}

static inline LY_DATA_TYPE
srepo_dat_value_type(const struct lyd_value * value)
{
	srepo_assert(value);
	srepo_assert(value->realtype);

	return value->realtype->basetype;
}

static inline bool
srepo_dat_value_as_bool(const struct lyd_value * value)
{
	srepo_assert(srepo_dat_value_type(value) == LY_TYPE_BOOL);

	return (bool)value->boolean;
}

static inline bool
srepo_dat_node_as_bool(const struct lyd_node * node)
{
	return srepo_dat_value_as_bool(srepo_dat_node_value(node));
}

extern sr_error_t
srepo_dat_node_dflt_as_bool(const struct lyd_node * node, bool * value)
	__srepo_export;

static inline const char *
srepo_dat_node_as_str(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(srepo_dat_node_type(node) & LYD_NODE_TERM);

	return lyd_get_value(node);
}

static inline const char *
srepo_dat_node_dflt_as_str(const struct lyd_node * node)
{
	return srepo_dat_node_dflt(node);
}

/******************************************************************************
 * YANG data node manipulation.
 ******************************************************************************/

extern sr_error_t
srepo_dat_change_bypath(sr_session_ctx_t * session,
                        const char *       path,
                        const char *       value,
                        const char *       origin,
                        uint32_t           flags)
	__srepo_nonull(1, 2, 3) __srepo_export;

extern sr_error_t
srepo_dat_vchangef_bypath(sr_session_ctx_t * session,
                          const char *       path,
                          const char *       origin,
                          uint32_t           flags,
                          const char *       format,
                          va_list            args)
	__srepo_nonull(1, 2, 5) __printf(5, 0) __srepo_export;

static inline __srepo_nonull(1, 2, 5) __printf(5, 6)
sr_error_t
srepo_dat_changef_bypath(sr_session_ctx_t * session,
                         const char *       path,
                         const char *       origin,
                         uint32_t           flags,
                         const char *       format,
                         ...)
{
	srepo_assert(session);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(!(flags & ~(SR_EDIT_DEFAULT |
	                         SR_EDIT_NON_RECURSIVE |
	                         SR_EDIT_STRICT |
	                         SR_EDIT_ISOLATE)));
	srepo_assert(format);
	srepo_assert(format[0]);

	va_list    args;
	sr_error_t ret;

	va_start(args, format);
	ret = srepo_dat_vchangef_bypath(
		session, path, origin, flags, format, args);
	va_end(args);

	return ret;
}

extern sr_error_t
srepo_dat_new_node(const struct ly_ctx * context,
                   struct lyd_node *     parent,
                   const char *          path,
                   const char *          value,
                   uint32_t              options,
                   struct lyd_node **    nevv)
	__srepo_nonull(3) __srepo_export;

static inline __srepo_nonull(3)
sr_error_t
srepo_dat_create_container(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    container)
{
	srepo_assert(context || parent);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(parent || (path[0] == '/'));

	return srepo_dat_new_node(context, parent, path, NULL, 0, container);
}

static inline __srepo_nonull(3)
sr_error_t
srepo_dat_create_list_ent(const struct ly_ctx * context,
                          struct lyd_node *     parent,
                          const char *          path,
                          struct lyd_node **    entry)
{
	srepo_assert(context || parent);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(parent || (path[0] == '/'));

	return srepo_dat_new_node(context, parent, path, NULL, 0, entry);
}

extern sr_error_t
srepo_dat_create_list_keyent(const struct ly_ctx * context,
                             struct lyd_node *     parent,
                             const char *          path,
                             const char *          key,
                             const char *          value,
                             struct lyd_node **    entry)
	__srepo_nonull(3, 4, 5) __srepo_export;

extern sr_error_t
srepo_dat_create_leaf(struct lyd_node *  parent,
                      const char *       path,
                      const char *       value,
                      struct lyd_node ** leaf)
	__srepo_export;

extern sr_error_t
srepo_dat_create_leaf_vprintf(struct lyd_node *  parent,
                              const char *       path,
                              struct lyd_node ** leaf,
                              const char *       format,
                              va_list            args)
	__srepo_export;

static inline sr_error_t
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

#define SREPO_DAT_IMPLICIT_OPTS \
	(LYD_IMPLICIT_NO_STATE | \
	 LYD_IMPLICIT_NO_CONFIG | \
	 LYD_IMPLICIT_OUTPUT | \
	 LYD_IMPLICIT_NO_DEFAULTS)

extern sr_error_t
srepo_dat_new_implicit(struct lyd_node *  tree,
                       uint32_t           options,
                       struct lyd_node ** diff)
	__srepo_export;

static inline void
srepo_dat_free_tree(struct lyd_node * tree)
{
	lyd_free_tree(tree);
}

/**
 * Prepare a batch of changes for merging.
 *
 * @param[in] session Session to prepare the batch of changes for
 * @param[in] trees   List of top-level change / edit trees
 */
static inline sr_error_t
srepo_dat_merge_batch(sr_session_ctx_t * session, const struct lyd_node * trees)
{
	srepo_assert(session);
	srepo_assert(trees);

	sr_error_t ret;

	ret = sr_edit_batch(session, trees, "merge");
	srepo_assert(ret != SR_ERR_INVAL_ARG);

	return ret;
}

static inline sr_error_t
srepo_dat_merge_data_batch(sr_session_ctx_t * session, const sr_data_t * data)
{
	srepo_assert(session);
	srepo_assert(data);
	srepo_assert(data->tree);

	return srepo_dat_merge_batch(session, data->tree);
}

/**
 * Prepare a batch of changes for replacement.
 *
 * @param[in] session Session to prepare the batch of changes for
 * @param[in] trees   List of top-level change / edit trees
 */
static inline sr_error_t
srepo_dat_replace_batch(sr_session_ctx_t *      session,
                        const struct lyd_node * trees)
{
	srepo_assert(session);
	srepo_assert(trees);

	sr_error_t ret;

	ret = sr_edit_batch(session, trees, "replace");
	srepo_assert(ret != SR_ERR_INVAL_ARG);

	return ret;
}

static inline sr_error_t
srepo_dat_replace_data_batch(sr_session_ctx_t * session, const sr_data_t * data)
{
	srepo_assert(session);
	srepo_assert(data);
	srepo_assert(data->tree);

	return srepo_dat_replace_batch(session, data->tree);
}

/******************************************************************************
 * Searching YANG data nodes / trees.
 ******************************************************************************/

extern sr_error_t
srepo_dat_find_path(const struct lyd_node * tree,
                    const char *            path,
                    struct lyd_node **      node)
	__srepo_export;

extern sr_error_t
srepo_dat_find_vpathf(const struct lyd_node * tree,
                      struct lyd_node **      node,
                      const char *            format,
                      va_list                 args)
	__srepo_export;

static inline sr_error_t
srepo_dat_find_pathf(const struct lyd_node * tree,
                     struct lyd_node **      node,
                     const char *            format,
                     ...)
{
	srepo_assert(tree);
	srepo_assert(node);
	srepo_assert(format);
	srepo_assert(format[0]);

	va_list    args;
	sr_error_t ret;

	va_start(args, format);
	ret = srepo_dat_find_vpathf(tree, node, format, args);
	va_end(args);

	return ret;
}

/******************************************************************************
 * Loading YANG data nodes / trees.
 ******************************************************************************/

#define srepo_dat_assert_flags(_flags) \
	srepo_assert(!((_flags) & ~(SR_OPER_NO_STATE | \
	                            SR_OPER_NO_CONFIG | \
	                            SR_OPER_NO_SUBS | \
	                            SR_OPER_NO_STORED | \
	                            SR_OPER_WITH_ORIGIN | \
	                            SR_OPER_NO_POLL_CACHED | \
	                            SR_OPER_NO_RUN_CACHED | \
	                            SR_OPER_NO_PUSH_NP_CONT | \
	                            SR_OPER_NO_NEW_CHANGES))); \
	srepo_assert(((_flags) & (SR_OPER_NO_STATE | SR_OPER_NO_CONFIG)) != \
	             (SR_OPER_NO_STATE | SR_OPER_NO_CONFIG))

/* Iterate over a list of YANG trees. */
#define srepo_dat_foreach_tree(_tree, _node) \
	LY_LIST_FOR(_tree, _node)

/* Iterate over a list of YANG data trees. */
#define srepo_dat_foreach_data_tree(_data, _node) \
	srepo_dat_foreach_tree((_data)->tree, _node)

/* Iterate over each node child. */
#define srepo_dat_foreach_child(_node, _child) \
	LY_LIST_FOR(lyd_child(_node), _child)

/* Iterate over each data tree child node. */
#define srepo_dat_foreach_data_child(_data, _child) \
	srepo_dat_foreach_child((_data)->tree, _child)

extern sr_error_t
srepo_dat_load_data(sr_session_ctx_t * session,
                    const char *       xpath,
                    unsigned int       depth,
                    sr_get_oper_flag_t flags,
                    sr_data_t **       data)
	__srepo_export;

static inline void
srepo_dat_release_data(sr_data_t * data)
{
	srepo_assert(data);

	sr_release_data(data);
}

static inline sr_error_t
srepo_dat_load_node(sr_session_ctx_t * session,
                    const char *       xpath,
                    sr_data_t **       data)
{
	srepo_assert(session);
	srepo_assert(xpath);
	srepo_assert(xpath[0]);
	srepo_assert(data);

	sr_error_t err;

	err = sr_get_node(session, xpath, 0, data);
	if (err != SR_ERR_OK)
		return err;

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

extern sr_error_t
srepo_dat_print_stdio_data(const sr_data_t * data,
                           LYD_FORMAT        format,
                           FILE *            stdio)
	__srepo_export;

extern sr_error_t
srepo_dat_print_data(const sr_data_t * data,
                     LYD_FORMAT        format,
                     struct ly_out *   printer)
	__srepo_export;

extern sr_error_t
srepo_open_stdio_print(struct ly_out ** printer, FILE * stdio)
	__srepo_export;

extern void
srepo_close_stdio_print(struct ly_out * printer)
	__srepo_export;

#endif /* defined(CONFIG_SREPO_PRINT) */

#endif /* _SREPO_DATA_H */
