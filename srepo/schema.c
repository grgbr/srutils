#include "srutils/srepo/schema.h"

/******************************************************************************
 * Libyang (compiled) schema node handling.
 ******************************************************************************/

char *
srepo_sch_node_path(const struct lysc_node * node)
{
	srepo_assert(node);

	char * path;
	char * pth;

	path = srepo_xpath_alloc();
	pth = lysc_path(node, LYSC_PATH_DATA, NULL, 0);
	if (pth) {
		srepo_assert(srepo_xpath_validate(path) > 0);
		return pth;
	}

	/* No more memory or SREPO_XPATH_SIZE not large enought ! */
	srepo_assert(0);
	srepo_abort();
}

/******************************************************************************
 * Libyang (compiled) schema extension handling.
 ******************************************************************************/

bool
srepo_sch_is_extension(const struct lysc_ext_instance * extension,
                       const char *                     identifier)
{
	srepo_assert(extension);
	srepo_assert(extension->def);
	srepo_assert(extension->def->module);
	srepo_assert(identifier);
	srepo_assert(identifier[0]);

	const struct lysc_ext *   def = extension->def;
	const struct lys_module * mod = def->module;

	return !strcmp(mod->name, "cly-extensions") &&
	       !strcmp(mod->ns, "urn:cly:yang:cly-extensions") &&
	       !(def->flags & LYS_STATUS_DEPRC) &&
	       !strcmp(def->name, identifier);
}

/******************************************************************************
 * Libyang modules / features handling.
 ******************************************************************************/

sr_error_t
srepo_sch_feature_status(const struct lys_module * module,
                         const char *              feature,
                         bool *                    enabled)
{
	srepo_assert(module);
	srepo_assert(feature);
	srepo_assert(feature[0]);
	srepo_assert(enabled);

	LY_ERR err;

	err = lys_feature_value(module, feature);
	switch (err) {
	case LY_SUCCESS:
		*enabled = true;
		break;

	case LY_ENOT:
		*enabled = false;
		break;

	case LY_ENOTFOUND:
		return SR_ERR_NOT_FOUND;

	default:
		break;
	}

	srepo_assert(0);

	return SR_ERR_INTERNAL;
}

const struct lys_module *
srepo_sch_find_module(const struct ly_ctx * context, const char * module)
{
	srepo_assert(context);
	srepo_assert(module);
	srepo_assert(module[0]);

	const struct lys_module * mod;

	mod = ly_ctx_get_module_implemented(context, module);
	if (mod && !sr_is_module_internal(mod))
		return mod;

	return NULL;
}
