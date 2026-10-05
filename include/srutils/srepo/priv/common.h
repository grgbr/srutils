#ifndef _SREPO_PRIV_COMMON_H
#define _SREPO_PRIV_COMMON_H

#include <srutils/priv/config.h>
#include <stroll/cdefs.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __srepo_export __export_public

#if defined(CONFIG_SREPO_ASSERT)

#include <stroll/assert.h>

#define __srepo_nonull(...)

#define srepo_assert(_cond) \
	stroll_assert("srepo", _cond)

#else  /* !defined(CONFIG_SRPLUG_ASSERT) */

#define __srepo_nonull(...) \
	__nonull(__VA_ARGS__)

#define srepo_assert(_cond)

#endif /* defined(CONFIG_SREPO_ASSERT) */

static inline __srepo_nonull(1) __warn_result
ssize_t
srepo_validate_strlen(const char * string, size_t size)
{
	srepo_assert(string);
	srepo_assert(size);
	srepo_assert(size <= SSIZE_MAX);

	size_t len;

	len = strnlen(string, size);
	if (len)
		return (len < size) ? (ssize_t)len : (ssize_t)-ENAMETOOLONG;
	else
		return (ssize_t)-ENODATA;
}

static inline __noreturn
void
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

static inline __alloc_size(1) \
              __returns_nonull \
              __warn_result
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
