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

static inline __srepo_nonull(1, 2)
const struct lysc_node *
srepo_sch_find_node(const struct lysc_node * tree, const char * path)
{
	srepo_assert(tree);
	srepo_assert(srepo_xpath_validate(path) > 0);

	return lys_find_path(NULL, tree, path, 0);
}

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

extern const struct lys_module *
srepo_sch_find_module(const struct ly_ctx * context, const char * module)
	__srepo_nonull(1, 2) __srepo_export;

#endif /* _SREPO_SCHEMA_H */
