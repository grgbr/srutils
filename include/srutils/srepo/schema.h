#ifndef _SREPO_SCHEMA_H
#define _SREPO_SCHEMA_H

#include <srutils/srepo/common.h>
#include <stdbool.h>

extern sr_error_t
srepo_sch_feature_status(const struct lys_module * module,
                         const char *              feature,
                         bool *                    enabled)
	__srepo_export;

extern const struct lys_module *
srepo_sch_find_module(const struct ly_ctx * context, const char * module)
	__srepo_export;

#endif /* _SREPO_SCHEMA_H */
