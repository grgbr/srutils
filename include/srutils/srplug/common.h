#ifndef _SRPLUG_COMMON_H
#define _SRPLUG_COMMON_H

#include <srutils/priv/config.h>
#include <srutils/srepo/common.h>
#include <stdbool.h>

#define __srplug_export __export_public

#if defined(CONFIG_SRPLUG_ASSERT)

#include <stroll/assert.h>

#define __srplug_nonull(...)

#define srplug_assert(_cond) \
	stroll_assert("srplug", _cond)

#else  /* !defined(CONFIG_SRPLUG_ASSERT) */

#define __srplug_nonull(...) \
	__nonull(__VA_ARGS__)

#define srplug_assert(_cond) \
	do { } while (0)

#endif /* defined(CONFIG_SRPLUG_ASSERT) */

#if defined(CONFIG_SRPLUG_DEBUG)

extern sr_error_t
srplug_change_debug(const sr_session_ctx_t * session,
                    const struct lyd_node *  node,
                    sr_change_oper_t         oper,
                    const char *             old,
                    void *                   data)
	__srplug_nonull(1, 2) __srplug_export;

#else  /* !defined(CONFIG_SRPLUG_DEBUG) */

static inline __nonull(1, 2)
sr_error_t
srplug_change_debug(const sr_session_ctx_t * session __unused,
                    const struct lyd_node *  node __unused,
                    sr_change_oper_t         oper __unused,
                    const char *             old __unused,
                    void *                   data __unused)
{
	return SR_ERR_OK;
}

#endif /* defined(CONFIG_SRPLUG_DEBUG) */

/**
 * Configuration data change handler function signature.
 */
typedef sr_error_t srplug_handle_change_fn(sr_session_ctx_t *,
                                           const struct lyd_node *,
                                           sr_change_oper_t,
                                           const char *,
                                           void *);

/**
 * Iterate over configuration data changes related to the XPATH given in
 * argument and handle them.
 */
extern sr_error_t
srplug_handle_changes(sr_session_ctx_t *        session,
                      const char *              xpath,
                      srplug_handle_change_fn * handle,
                      void *                    data)
	__srplug_nonull(1, 2, 3) __srplug_export;

/**
 * Configuration data change HaNDLeR.
 */
struct srplug_change_hndlr {
	const char *               name;
	const struct srplug_feat * feature;
	srplug_handle_change_fn *  handle;
};

#define SRPLUG_CHANGE_HNDLR(_name, _feat, _handle) \
	{ \
		.name    = _name, \
		.feature = _feat, \
		.handle  = _handle, \
	}

/**
 * Process configuration data changes related to XPATH direct children according
 * to handlers given in argument.
 */
extern sr_error_t
srplug_process_child_changes(sr_session_ctx_t *                 session,
                             const char *                       xpath,
                             const struct srplug_change_hndlr * handlers,
                             unsigned int                       nr,
                             void *                             data)
	__srplug_nonull(1, 2, 3) __srplug_export;

struct srplug_change_sub {
	const char *               module;
	const char *               xpath;
	const struct srplug_feat * feature;
	sr_module_change_cb        on_change;
	uint32_t                   priority;
	uint32_t                   options;
};

struct srplug_oper_sub {
	const char *               module;
	const char *               xpath;
	const struct srplug_feat * feature;
	sr_oper_get_items_cb       on_get;
	uint32_t                   options;
};

struct srplug_rpc_sub {
	const char *               xpath;
	const struct srplug_feat * feature;
	sr_rpc_cb                  on_rpc;
	uint32_t                   priority;
	uint32_t                   options;
};

enum srplug_sub_kind {
	SRPLUG_CHANGE_SUB_KIND = 0,
	SRPLUG_OPER_SUB_KIND,
	SRPLUG_RPC_SUB_KIND,
	SRPLUG_SUB_KIND_NR
};

struct srplug_sub {
	enum srplug_sub_kind             kind;
	union {
		struct srplug_change_sub change;
		struct srplug_oper_sub   oper;
		struct srplug_rpc_sub    rpc;
	};
};

#define SRPLUG_CHANGE_SUB(_mod, _xpath, _feat, _on_change, _prio, _opts) \
	{ \
		.kind   = SRPLUG_CHANGE_SUB_KIND, \
		.change = { \
			.module    = _mod, \
			.xpath     = _xpath, \
			.feature   = _feat, \
			.on_change = _on_change, \
			.priority  = _prio, \
			.options   = _opts \
		} \
	}

#define SRPLUG_OPER_SUB(_mod, _xpath, _feat, _on_get, _prio, _opts) \
	{ \
		.kind = SRPLUG_OPER_SUB_KIND, \
		.oper = { \
			.module  = _mod, \
			.xpath   = _xpath, \
			.feature = _feat, \
			.on_get  = _on_get, \
			.options = _opts \
		} \
	}

#define SRPLUG_RPC_SUB(_mod, _xpath, _feat, _on_change, _prio, _opts) \
	{ \
		.kind = SRPLUG_RPC_SUB_KIND, \
		.rpc  = { \
			.xpath    = _xpath, \
			.feature  = _feat, \
			.on_rpc   = _on_rpc, \
			.priority = _prio, \
			.options  = _opts \
		} \
	}

struct srplug_feat {
	const char * const name;
	bool               on;
};

#define SRPLUG_FEAT_SETUP(_name) \
	{ \
		.name   = _name, \
		.on     = false \
	}

extern sr_error_t
srplug_probe_feature(const struct ly_ctx * context,
                     const char *          module,
                     struct srplug_feat *  feature)
	__srplug_nonull(1, 2, 3) __srplug_export;

const struct lys_module *
srplug_find_module(const struct ly_ctx * context, const char * module)
	__srplug_nonull(1, 2) __srplug_export;

#endif /* _SRPLUG_COMMON_H */
