#ifndef _SREPO_PRIV_COMMON_H
#define _SREPO_PRIV_COMMON_H

#include <srutils/priv/config.h>
#include <stroll/cdefs.h>
#include <stdlib.h>

static inline __srepo_nonull(1) __warn_result
ssize_t
srepo_validate_strlen(const char * string, size_t size)
{
	srepo_assert(string);
	srepo_assert(size);

	size_t len;

	len = strnlen(xpath, size);
	if (len)
		return (len < size) ? len : -ENAMETOOLONG;
	else
		return -ENODATA;
}

static void __noreturn
srepo_abort(void)
{
	abort();
}

static inline
void
srepo_free(void * data)
{
	free(data);
}

static __malloc(srepo_free, 1) __warn_result
void *
srepo_malloc(size_t size)
{
	srepo_assert(size);

	void * data;

	data = malloc(size);
	if (!data)
		srepo_abort();

	return data;
}

#endif /* _SREPO_PRIV_COMMON_H */
