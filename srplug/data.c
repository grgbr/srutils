#include "srutils/srplug/data.h"

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

	srplug_pnode_notice(parent, path, "cannot create container");

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

	srplug_pnode_notice(parent, path, "cannot create list entry");

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

	srplug_pnode_notice(parent, path, "cannot create keyed list entry");

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

	srplug_pnode_notice(parent, path, "cannot create leaf");

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

	srplug_node_notice(tree, "cannot populate with default nodes");

	return ret;
}

/******************************************************************************
 * Searching for / loading YANG data nodes / trees.
 ******************************************************************************/

int
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
