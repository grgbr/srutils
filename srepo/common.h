#ifndef _SREPO_INTERN_COMMON_H
#define _SREPO_INTERN_COMMON_H

#include "srutils/srepo/common.h"

#define __srepo_intern __export_intern

extern sr_error_t
srepo_ly_error(LY_ERR error)
	__srepo_intern;

extern sr_error_t
srepo_sys_error(int error)
	__srepo_intern;

#endif /* _SREPO_INTERN_COMMON_H */
