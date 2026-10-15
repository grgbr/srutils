#ifndef _SRPLUG_INTERN_COMMON_H
#define _SRPLUG_INTERN_COMMON_H

#include <srutils/srplug/common.h>

static inline void __noreturn
srplug_abort(void)
{
	abort();
}

static inline void *
srplug_malloc(size_t size)
{
	srplug_assert(size);

	void * data;

	data = malloc(size);
	if (!data)
		srplug_abort();

	return data;
}

#endif /* _SRPLUG_INTERN_COMMON_H */
