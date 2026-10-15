#ifndef _SREPO_PRIV_DATA_H
#define _SREPO_PRIV_DATA_H

#include <srutils/srepo/log.h>

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

#endif /* _SREPO_PRIV_DATA_H */
