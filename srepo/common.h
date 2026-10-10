#ifndef _SREPO_INTERN_COMMON_H
#define _SREPO_INTERN_COMMON_H

#include "srutils/srepo/common.h"

#define __srepo_intern __export_intern

#define srepo_dat_assert_get_flags(_flags) \
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
srepo_ly_error(LY_ERR error)
	__srepo_intern;

#endif /* _SREPO_INTERN_COMMON_H */
