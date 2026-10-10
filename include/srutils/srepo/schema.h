#ifndef _SREPO_SCHEMA_H
#define _SREPO_SCHEMA_H

#include <srutils/srepo/common.h>
#include <srutils/srepo/xpath.h>

/******************************************************************************
 * Libyang (compiled) schema node handling.
 ******************************************************************************/

static inline __srepo_nonull(1)
uint16_t
srepo_sch_conf_flags(const struct lysc_node * node)
{
	srepo_assert(node);

	return node->flags & LYS_CONFIG_MASK;
}

static inline __srepo_nonull(1)
uint16_t
srepo_sch_status_flags(const struct lysc_node * node)
{
	srepo_assert(node);

	return node->flags & LYS_STATUS_MASK;
}

static inline __srepo_nonull(1)
const struct lysc_node *
srepo_sch_parent(const struct lysc_node * node)
{
	srepo_assert(node);

	return lysc_data_parent(node);
}

static inline __srepo_nonull(1)
const struct lysc_node *
srepo_sch_child(const struct lysc_node * node)
{
	srepo_assert(node);

	return lysc_node_child(node);
}

extern char *
srepo_sch_node_path(const struct lysc_node * node)
	__srepo_nonull(1) __returns_nonull __srepo_export;

/******************************************************************************
 * Searching / browsing YANG (compiled) schema nodes / trees.
 ******************************************************************************/

/* Iterate over each (compiled) schema node child. */
#define srepo_sch_foreach_child(_node, _child) \
	LY_LIST_FOR(srepo_sch_child(_node), _child)

typedef int srepo_sch_visit_fn(const struct lysc_node *,
                               enum srepo_walk_event,
                               void *);

/**
 * Perform a depth first traversal of the Yang (compiled) schema tree given in
 * argument.
 */
extern sr_error_t
srepo_sch_walk_tree(const struct lysc_node * tree,
                    srepo_sch_visit_fn *     visit,
                    void *                   data)
	__srepo_nonull(1, 2) __srepo_export;

#if defined(CONFIG_SREPO_PRINT)

/**
 * Print YANG specification for the libyang schema tree given in argument
 * according to RFC 7950.
 *
 * When given as `true', the `nosub' argument requests the printing of the
 * YANG top-level `module' statement only, i.e., do not print the module's
 * sub-statements.
 */
extern sr_error_t
srepo_sch_print_node_yang(struct ly_out *          printer,
                          const struct lysc_node * tree,
                          bool                     nosub)
	__srepo_nonull(1, 2) __srepo_export;

/**
 * Print the YANG tree diagram for the libyang schema node given in argument
 * according to RFC 8340.
 */
extern sr_error_t
srepo_sch_print_node_diag(struct ly_out *          printer,
                          const struct lysc_node * tree,
                          unsigned int             cols_nr)
	__srepo_nonull(1, 2) __srepo_export;

#endif /* defined(CONFIG_SREPO_PRINT) */

static inline __srepo_nonull(1, 2)
const struct lysc_node *
srepo_sch_find_node(const struct lysc_node * tree, const char * path)
{
	srepo_assert(tree);
	srepo_assert(srepo_xpath_validate(path) > 0);

	return lys_find_path(NULL, tree, path, 0);
}

/******************************************************************************
 * Loading Yang data from (compiled) schema nodes.
 ******************************************************************************/

/**
 * Load a (possibly partial) subtree identified thanks to a schema node.
 */
extern sr_error_t
srepo_sch_load_data(sr_session_ctx_t *       session,
                    const struct lysc_node * node,
                    unsigned int             depth,
                    sr_get_oper_flag_t       flags,
                    sr_data_t **             data)
	__srepo_nonull(1, 2, 5) __srepo_export;

/**
 * Load an entire data subtree identified thanks to a schema node.
 */
extern sr_error_t
srepo_sch_load_subtree(sr_session_ctx_t *       session,
                       const struct lysc_node * node,
                       sr_data_t **             data)
	__srepo_nonull(1, 2, 3) __srepo_export;

/**
 * Load a single data node identified thanks to a schema node.
 */
extern sr_error_t
srepo_sch_load_node(sr_session_ctx_t *       session,
                    const struct lysc_node * node,
                    sr_data_t **             data)
	__srepo_nonull(1, 2, 3) __srepo_export;

/******************************************************************************
 * Libyang (compiled) schema extension handling.
 ******************************************************************************/

/**
 * Iterate over extension instances.
 *
 * @param[in]    _ext_array  a libyang @ref sizedarrays of ::lysc_ext_instance
 *                           extension instance structures
 * @param[inout] _ext        pointer to the current ::lysc_ext_instance
 *                           extension instance structure
 */
#define srepo_sch_foreach_extension(_ext_array, _ext) \
	LY_ARRAY_FOR(_ext_array, struct lysc_ext_instance, _ext)

/**
 * Tell wether the given extension instance is one of our own `cli-extension`
 * extension directives or not.
 *
 * @param[in] extension  pointer to the extension instance to test
 * @param[in] identifier extension directive
 *
 * When installed and enabled, the cli extension plugin implements support for 
 * YANG syntax extensions defined into the `cli-extension.yang` module.
 *
 * This function tests wether or not the extension instance given as @p
 * extension is a `cli-extension` defined statement and which identifier is
 * given as @p identifier.
 */
extern bool
srepo_sch_is_extension(const struct lysc_ext_instance * extension,
                       const char *                     identifier)
	__srepo_nonull(1, 2) __srepo_export;

/******************************************************************************
 * Libyang modules / features handling.
 ******************************************************************************/

extern sr_error_t
srepo_sch_feature_status(const struct lys_module * module,
                         const char *              feature,
                         bool *                    enabled)
	__srepo_nonull(1, 2, 3) __srepo_export;

static inline __srepo_nonull(1) __returns_nonull
char *
srepo_sch_module_path(const struct lys_module * module)
{
	srepo_assert(module);
	srepo_assert(module->name[0]);

	char *  path;
	ssize_t len;

	len = srepo_xpath_createf(&path, "/%s", module->name);
	srepo_assert(len > 0);

	return path;
}

#if defined(CONFIG_SREPO_PRINT)

/**
 * Print YANG specification for the libyang module which name is given in
 * argument according to RFC 7950.
 *
 * When given as `true', the `nosub' argument requests the printing of the
 * YANG top-level `module' statement only, i.e., do not print the module's
 * sub-statements.
 */
extern sr_error_t
srepo_sch_print_module_yang(struct ly_out *           printer,
                            const struct lys_module * module,
                            bool                      nosub)
	__srepo_nonull(1, 2) __srepo_export;

/**
 * Print the YANG tree diagram for the libyang module which name is given in
 * argument according to RFC 8340.
 */
extern sr_error_t
srepo_sch_print_module_diag(struct ly_out *           printer,
                            const struct lys_module * module,
                            unsigned int              cols_nr)
	__srepo_nonull(1, 2) __srepo_export;

#endif /* defined(CONFIG_SREPO_PRINT) */

extern const struct lys_module *
srepo_sch_next_module(const struct ly_ctx * context, unsigned int * index)
	__srepo_nonull(1, 2) __srepo_export;

/**
 * Iterate over YANG implemented modules, skipping sysrepo / libyang internal
 * ones.
 *
 * Return compiled and (features) implemented, i.e. completely resolved modules
 * with top-level data node or extension instances only.
 */
#define cli_lys_foreach_module(_context, _index, _module) \
	for ((_index) = 0, \
	     (_module) = srepo_sch_next_module(_context, &(_index)); \
	     _module; \
	     (_module) = srepo_sch_next_module(_context, &(_index)))

/*
 * Perform a depth first traversal for the Libyang (compiled) schema tree of the
 * module given in argument.
 */
extern sr_error_t
srepo_sch_walk_module(const struct lys_module * module,
                      srepo_sch_visit_fn *      visit,
                      void *                    data)
	__srepo_nonull(1, 2) __srepo_export;

/**
 * Return YANG implemented module given by name, excluding sysrepo / libyang
 * internal ones.
 */
extern const struct lys_module *
srepo_sch_find_module(const struct ly_ctx * context, const char * module)
	__srepo_nonull(1, 2) __srepo_export;

#endif /* _SREPO_SCHEMA_H */
