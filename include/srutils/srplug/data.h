#ifndef _SRPLUG_DATA_H
#define _SRPLUG_DATA_H

#include <srutils/srplug/common.h>
#include <srutils/srepo/data.h>

/******************************************************************************
 * YANG xpath manipulation.
 ******************************************************************************/

extern char *
srplug_dat_path(const struct lyd_node * node);

/******************************************************************************
 * YANG data node value manipulation.
 ******************************************************************************/

extern sr_error_t
srplug_dat_create_container(const struct ly_ctx * context,
                            struct lyd_node *     parent,
                            const char *          path,
                            struct lyd_node **    container);

extern sr_error_t
srplug_dat_create_list_ent(const struct ly_ctx * context,
                           struct lyd_node *     parent,
                           const char *          path,
                           struct lyd_node **    entry);

extern sr_error_t
srplug_dat_create_list_keyent(const struct ly_ctx * context,
                              struct lyd_node *     parent,
                              const char *          path,
                              const char *          key,
                              const char *          value,
                              struct lyd_node **    entry);

extern sr_error_t
srplug_dat_create_leaf(struct lyd_node *  parent,
                       const char *       path,
                       const char *       value,
                       struct lyd_node ** leaf);

extern sr_error_t
srplug_dat_populate_defaults(struct lyd_node *  tree,
                             uint32_t           options,
                             struct lyd_node ** diff);

static inline void
srplug_dat_free_tree(struct lyd_node * tree)
{
	srepo_dat_free_tree(tree);
}

/******************************************************************************
 * Searching for / loading YANG data nodes / trees.
 ******************************************************************************/

extern int
srplug_dat_load_node(sr_session_ctx_t * session,
                     const char *       xpath,
                     sr_data_t **       data);

#endif /* _SRPLUG_DATA_H */
