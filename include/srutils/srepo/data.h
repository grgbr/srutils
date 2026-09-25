#ifndef _SREPO_DATA_H
#define _SREPO_DATA_H

#include <srutils/srepo/common.h>
#include <stdbool.h>

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

static inline const struct lyd_value *
srepo_dat_node_value(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(node->schema->nodetype & LYD_NODE_TERM);

	return &((const struct lyd_node_term *)node)->value;
}

static inline const char *
srepo_dat_node_dflt(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(node->schema->nodetype == LYS_LEAF);

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
srepo_dat_node_dflt_as_bool(const struct lyd_node * node, bool * value);

static inline const char *
srepo_dat_node_as_str(const struct lyd_node * node)
{
	srepo_assert(node);
	srepo_assert(node->schema);
	srepo_assert(node->schema->nodetype & LYD_NODE_TERM);

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

extern char *
srepo_dat_path(const struct lyd_node * node);

extern sr_error_t
srepo_dat_create_container(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    container);

extern sr_error_t
srepo_dat_create_list_ent(const struct ly_ctx * context,
                          struct lyd_node *     parent,
                          const char *          path,
                          struct lyd_node **    entry);

extern sr_error_t
srepo_dat_create_list_keyent(const struct ly_ctx * context,
                             struct lyd_node *     parent,
                             const char *          path,
                             const char *          key,
                             const char *          value,
                             struct lyd_node **    entry);

extern sr_error_t
srepo_dat_create_leaf(struct lyd_node *  parent,
                      const char *       path,
                      const char *       value,
                      struct lyd_node ** leaf);

#define SREPO_DAT_IMPLICIT_OPTS \
	(LYD_IMPLICIT_NO_STATE | \
	 LYD_IMPLICIT_NO_CONFIG | \
	 LYD_IMPLICIT_OUTPUT | \
	 LYD_IMPLICIT_NO_DEFAULTS)

extern sr_error_t
srepo_dat_new_implicit(struct lyd_node *  tree,
                       uint32_t           options,
                       struct lyd_node ** diff);

static inline void
srepo_dat_free_tree(struct lyd_node * tree)
{
	lyd_free_tree(tree);
}

/******************************************************************************
 * Searching for / loading YANG data nodes / trees.
 ******************************************************************************/

static inline int
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

#endif /* _SREPO_DATA_H */
