#ifndef _SREPO_DATA_H
#define _SREPO_DATA_H

#include <srutils/srepo/common.h>
#include <stdbool.h>
#include <stdarg.h>

/******************************************************************************
 * YANG xpath manipulation.
 ******************************************************************************/

static inline char *
srepo_dat_path(const struct lyd_node * node)
{
	srepo_assert(node);

	return lyd_path(node, LYD_PATH_STD, NULL, 0);
}

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

static inline uint16_t
srepo_dat_node_type(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);

	return node->schema->nodetype;
}

static inline const struct lyd_value *
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

static inline const char *
srepo_dat_node_name(const struct lyd_node * node)
{
	srepo_assert(node);

	return LYD_NAME(node);
}

extern sr_error_t
srepo_dat_create_container(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    container)
	__srepo_export;

extern sr_error_t
srepo_dat_create_list_ent(const struct ly_ctx * context,
                          struct lyd_node *     parent,
                          const char *          path,
                          struct lyd_node **    entry)
	__srepo_export;

extern sr_error_t
srepo_dat_create_list_keyent(const struct ly_ctx * context,
                             struct lyd_node *     parent,
                             const char *          path,
                             const char *          key,
                             const char *          value,
                             struct lyd_node **    entry)
	__srepo_export;

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

extern sr_error_t
srepo_dat_create_leaf_printf(struct lyd_node *  parent,
                             const char *       path,
                             struct lyd_node ** leaf,
                             const char *       format,
                             ...)
	__srepo_export;

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

/******************************************************************************
 * Searching for / loading YANG data nodes / trees.
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

extern sr_error_t
srepo_dat_load(sr_session_ctx_t * session,
               const char *       xpath,
               unsigned int       depth,
               sr_get_oper_flag_t flags,
               sr_data_t **       data)
	__srepo_export;

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
