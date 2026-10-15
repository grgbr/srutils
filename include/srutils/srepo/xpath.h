#ifndef _SREPO_XPATH_H
#define _SREPO_XPATH_H

#include <srutils/srepo/common.h>
#include <sysrepo/xpath.h>

/******************************************************************************
 * XPATH parsing logic.
 ******************************************************************************/

#define SREPO_XPATH_SIZE (4096U)

static inline __srepo_nonull(1) __warn_result
ssize_t
srepo_xpath_validate(const char * xpath)
{
	return srepo_validate_strlen(xpath, SREPO_XPATH_SIZE);
}

#define SREPO_XPATH_NODE_SIZE (256U)

static inline __srepo_nonull(1) __warn_result
ssize_t
srepo_xpath_validate_node(const char * node)
{
	return srepo_validate_strlen(node, SREPO_XPATH_NODE_SIZE);
}

#define SREPO_XPATH_KEY_SIZE (256U)

static inline __srepo_nonull(1) __warn_result
ssize_t
srepo_xpath_validate_key(const char * key)
{
	return srepo_validate_strlen(key, SREPO_XPATH_KEY_SIZE);
}

static inline __srepo_nonull(1, 2, 3, 4) __warn_result
const char *
srepo_xpath_key_value(char *           xpath,
                      const char *     node,
                      const char *     key,
                      sr_xpath_ctx_t * context)
{
	srepo_assert(srepo_xpath_validate(xpath) > 0);
	srepo_assert(srepo_xpath_validate_node(node) > 0);
	srepo_assert(srepo_xpath_validate_key(key) > 0);
	srepo_assert(context);

	return sr_xpath_key_value(xpath, node, key, context);
}

static inline __srepo_nonull(1)
void
srepo_xpath_recover(sr_xpath_ctx_t * context)
{
	srepo_assert(context);

	sr_xpath_recover(context);
}

/******************************************************************************
 * XPATH build logic.
 ******************************************************************************/

extern ssize_t
srepo_xpath_concat(char * __restrict destination,
                   size_t            destnation_length,
                   char * __restrict source,
                   size_t            source_length)
	__srepo_nonull(1, 3) __srepo_export;

extern ssize_t
srepo_xpath_vprintf(char * __restrict       xpath,
                    size_t                  length,
                    const char * __restrict format,
                    va_list                 args)
	__srepo_nonull(1, 3) __printf(3, 0) __srepo_export;

extern ssize_t
srepo_xpath_printf(char * __restrict       xpath,
                   size_t                  length,
                   const char * __restrict format,
                   ...)
	__srepo_nonull(1, 3) __printf(3, 4) __srepo_export;

static __returns_nonull __warn_result
void *
srepo_xpath_alloc(void)
{
	return srepo_malloc(SREPO_XPATH_SIZE);
}

extern char *
srepo_xpath_create(const char * __restrict xpath, size_t length)
	__srepo_nonull(1) \
	__returns_nonull \
	__warn_result \
	__srepo_export;

extern ssize_t
srepo_xpath_vcreatef(char ** __restrict      xpath,
                     const char * __restrict format,
                     va_list                 args)
	__srepo_nonull(1, 2) __printf(2, 0) __srepo_export;

static inline __srepo_nonull(1) __printf(2, 3)
ssize_t
srepo_xpath_createf(char ** __restrict      xpath,
                    const char * __restrict format,
                    ...)
{
	srepo_assert(xpath);
	srepo_assert(format);
	srepo_assert(format[0]);

	va_list args;
	int     len;

	va_start(args, format);
	len = srepo_xpath_vcreatef(xpath, format, args);
	va_end(args);

	return (ssize_t)len;
}

#endif /* _SREPO_XPATH_H */
