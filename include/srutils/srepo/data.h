#ifndef _SREPO_DATA_H
#define _SREPO_DATA_H

#include <srutils/srepo/priv/data.h>
#include <srutils/srepo/xpath.h>
#include <stdbool.h>

/******************************************************************************
 * YANG data node properties handling.
 ******************************************************************************/

extern const char *
srepo_dat_node_name(const struct lyd_node * node)
	__srepo_nonull(1) __warn_result __srepo_export;

static inline uint16_t
srepo_dat_node_type(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);

	return node->schema->nodetype;
}

extern char *
srepo_dat_node_path(const struct lyd_node * node)
	__srepo_nonull(1) __warn_result __srepo_export;

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

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

#define SREPO_DAT_IMPLICIT_OPTS \
	(LYD_IMPLICIT_NO_STATE | \
	 LYD_IMPLICIT_NO_CONFIG | \
	 LYD_IMPLICIT_OUTPUT | \
	 LYD_IMPLICIT_NO_DEFAULTS)

extern sr_error_t
srepo_dat_new_dflt_nodes(struct lyd_node *  tree,
                         uint32_t           options,
                         struct lyd_node ** diff)
	__srepo_nonull(1) __srepo_export;

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

static inline __srepo_nonull(1, 2, 3) __srepo_nonull(3)
sr_error_t
srepo_dat_create_leaf(struct lyd_node *  parent,
                      const char *       path,
                      const char *       value,
                      struct lyd_node ** leaf)
{
	srepo_assert(parent);
	srepo_assert(srepo_xpath_validate(path) > 0);
	srepo_assert(value);
	srepo_assert(value[0]);

	return srepo_dat_new_node(NULL, parent, path, value, 0, leaf);
}

extern sr_error_t
srepo_dat_vcreatef_leaf(struct lyd_node *  parent,
                        const char *       path,
                        struct lyd_node ** leaf,
                        const char *       format,
                        va_list            args)
	__srepo_nonull(1, 2, 4) __printf(4, 0) __srepo_export;

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
	ret = srepo_dat_vcreatef_leaf(parent, path, leaf, format, args);
	va_end(args);

	return ret;
}

static inline
void
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
static inline __srepo_nonull(1, 2)
sr_error_t
srepo_dat_merge_batch(sr_session_ctx_t * session, const struct lyd_node * trees)
{
	srepo_assert(session);
	srepo_assert(trees);

	sr_error_t ret;

	ret = sr_edit_batch(session, trees, "merge");
	srepo_assert(ret != SR_ERR_INVAL_ARG);

	return ret;
}

static inline __srepo_nonull(1, 2)
sr_error_t
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
static inline __srepo_nonull(1, 2)
sr_error_t
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

static inline __srepo_nonull(1, 2)
sr_error_t
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
srepo_dat_find_node(const struct lyd_node * tree,
                    const char *            path,
                    struct lyd_node **      node)
	__srepo_nonull(1, 2, 3) __srepo_export;

extern sr_error_t
srepo_dat_vfindf_node(const struct lyd_node * tree,
                      struct lyd_node **      node,
                      const char *            format,
                      va_list                 args)
	__srepo_nonull(1, 2, 3) __printf(3, 0) __srepo_export;

static inline __srepo_nonull(1, 2, 3) __printf(3, 4)
sr_error_t
srepo_dat_findf_node(const struct lyd_node * tree,
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
	ret = srepo_dat_vfindf_node(tree, node, format, args);
	va_end(args);

	return ret;
}

/******************************************************************************
 * Loading YANG data nodes / trees.
 ******************************************************************************/

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
	__srepo_nonull(1, 2, 5) __srepo_export;

extern sr_error_t
srepo_dat_load_node(sr_session_ctx_t * session,
                    const char *       xpath,
                    sr_data_t **       data)
	__srepo_nonull(1, 2, 3) __srepo_export;

static inline __srepo_nonull(1)
void
srepo_dat_release_data(sr_data_t * data)
{
	srepo_assert(data);

	sr_release_data(data);
}

/******************************************************************************
 * Debugging / printing / logging YANG data nodes / trees.
 ******************************************************************************/

#if defined(CONFIG_SREPO_PRINT)

extern
sr_error_t
srepo_dat_print_stdio_data(const sr_data_t * data,
                           LYD_FORMAT        format,
                           FILE *            stdio)
	__srepo_nonull(1, 3) __srepo_export;

extern sr_error_t
srepo_dat_print_data(const sr_data_t * data,
                     LYD_FORMAT        format,
                     struct ly_out *   printer)
	__srepo_nonull(1, 3) __srepo_export;

#endif /* defined(CONFIG_SREPO_PRINT) */

#if defined(CONFIG_SREPO_LOG)

static inline __srepo_nonull(1, 3, 4) __printf(4, 5)
void
srepo_dat_log_node(const struct lyd_node * __restrict node,
                   enum elog_severity                 severity,
                   const char * __restrict            prefix,
                   const char * __restrict            format,
                   ...)
{
	srepo_assert(node);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	if (srepo_logger) {
		va_list args;

		va_start(args, format);
		srepo_dat_vlog_node(node, severity, prefix, format, args);
		va_end(args);
	}
}

#define srepo_dat_node_err(_node, _fmt, ...) \
	srepo_dat_log_node(_node, \
	                   ELOG_ERR_SEVERITY, \
	                   "srepo", \
	                   _fmt, \
	                   ## __VA_ARGS__)

#define srepo_dat_node_warn(_node, _fmt, ...) \
	srepo_dat_log_node(_node, \
	                   ELOG_WARNING_SEVERITY, \
	                   "srepo", \
	                   _fmt, \
	                   ## __VA_ARGS__)

#define srepo_dat_node_notice(_node, _fmt, ...) \
	srepo_dat_log_node(_node, \
	                   ELOG_NOTICE_SEVERITY, \
	                   "srepo", \
	                   _fmt, \
	                   ## __VA_ARGS__)

#define srepo_dat_node_info(_node, _fmt, ...) \
	srepo_dat_log_node(_node, \
	                   ELOG_INFO_SEVERITY, \
	                   "srepo", \
	                   _fmt, \
	                   ## __VA_ARGS__)

#if defined(CONFIG_SREPO_DEBUG)

#define srepo_dat_node_debug(_node, _fmt, ...) \
	srepo_dat_log_node(_node, \
	                   ELOG_DEBUG_SEVERITY, \
	                   "srepo", \
	                   _fmt, \
	                   ## __VA_ARGS__)

#endif /* defined(CONFIG_SREPO_DEBUG) */

static inline __srepo_nonull(1, 2, 4, 5) __printf(5, 6)
void
srepo_dat_log_sess_node(const sr_session_ctx_t * __restrict session,
                        const struct lyd_node * __restrict  node,
                        enum elog_severity                  severity,
                        const char * __restrict             prefix,
                        const char * __restrict             format,
                        ...)
{
	srepo_assert(session);
	srepo_assert(node);
	srepo_assert(prefix);
	srepo_assert(format);
	srepo_assert(format[0]);

	if (srepo_logger) {
		va_list args;

		va_start(args, format);
		srepo_dat_vlog_sess_node(session,
		                         node,
		                         severity,
		                         prefix,
		                         format,
		                         args);
		va_end(args);
	}
}

#define srepo_dat_sess_node_err(_sess, _node, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        ELOG_ERR_SEVERITY, \
	                        "srepo", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#define srepo_dat_sess_node_warn(_sess, _node, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        ELOG_WARNING_SEVERITY, \
	                        "srepo", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#define srepo_dat_sess_node_notice(_sess, _node, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        ELOG_NOTICE_SEVERITY, \
	                        "srepo", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#define srepo_dat_sess_node_info(_sess, _node, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        ELOG_INFO_SEVERITY, \
	                        "srepo", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#if defined(CONFIG_SREPO_DEBUG)

#define srepo_dat_sess_node_debug(_sess, _node, _fmt, ...) \
	srepo_dat_log_sess_node(_sess, \
	                        _node, \
	                        ELOG_DEBUG_SEVERITY, \
	                        "srepo", \
	                        _fmt, \
	                        ## __VA_ARGS__)

#endif /* defined(CONFIG_SREPO_DEBUG) */

#else  /* !defined(CONFIG_SREPO_LOG) */

static inline __srepo_nonull(1, 3, 4) __printf(4, 5)
void
srepo_dat_vlog_node(const struct lyd_node * __restrict node __unused,
                    enum elog_severity                 severity __unused,
                    const char * __restrict            prefix __unused,
                    const char * __restrict            format __unused,
                    va_list                            args __unused)
{
}

static inline __srepo_nonull(1, 2, 4, 5) __printf(5, 0)
void
srepo_dat_vlog_sess_node(const sr_session_ctx_t * __restrict session __unused,
                         const struct lyd_node * __restrict  node __unused,
                         enum elog_severity                  severity __unused,
                         const char * __restrict             prefix __unused,
                         const char * __restrict             format __unused,
                         va_list                             args __unused)
{
}

#define srepo_dat_node_err(_sess, _node, _fmt, ...)
#define srepo_dat_node_warn(_sess, _node, _fmt, ...)
#define srepo_dat_node_notice(_sess, _node, _fmt, ...)
#define srepo_dat_node_info(_sess, _node, _fmt, ...)
#if defined(CONFIG_SREPO_DEBUG)
#define srepo_dat_node_debug(_sess, _node, _fmt, ...)
#endif /* defined(CONFIG_SREPO_DEBUG) */

#endif /* defined(CONFIG_SREPO_LOG) */

#endif /* _SREPO_DATA_H */
