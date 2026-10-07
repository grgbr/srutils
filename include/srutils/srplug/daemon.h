#ifndef _SRPLUG_DAEMON_H
#define _SRPLUG_DAEMON_H

#if defined(_SRPLUG_THREAD_H)
#error daemon.h and thread.h header files are mutually exclusive !
#endif /* defined(_SRPLUG_THREAD_H) */

#include <srutils/srplug/common.h>
#include <utils/poll.h>
#include <utils/timer.h>
#include <elog/elog.h>
#include <sysrepo.h>

/******************************************************************************
 * Command line parsing.
 ******************************************************************************/

struct srplug_daemon_conf {
#if defined(CONFIG_SRPLUG_DAEMON_STDLOG)
	struct elog_stdio_conf  stdlog;
#endif /* defined(CONFIG_SRPLUG_STDLOG) */
#if defined(CONFIG_SRPLUG_DAEMON_SYSLOG)
	struct elog_syslog_conf syslog;
#endif /* defined(CONFIG_SRPLUG_SYSLOG) */
};

#if defined(CONFIG_SRPLUG_DAEMON_CONFIG)

extern struct srplug_daemon_conf *
srplug_daemon_alloc_conf(void)
	__srplug_export;

extern void
srplug_daemon_free_conf(struct srplug_daemon_conf * config)
	__srplug_export;

#else  /* !defined(CONFIG_SRPLUG_DAEMON_CONFIG) */

static inline struct srplug_daemon_conf *
srplug_daemon_alloc_conf(void)
{
	return NULL;
}

static inline void
srplug_daemon_free_conf(struct srplug_daemon_conf * config __unused)
{
}

#endif /* defined(CONFIG_SRPLUG_DAEMON_CONFIG) */

struct argp_state;
struct srplug_daemon_cmdln_opt;

typedef int
        srplug_cmdln_parse_fn(const struct srplug_daemon_cmdln_opt * option,
                              const char *                           argument,
                              struct argp_state *                    state,
                              struct srplug_daemon_conf *            config);

struct srplug_daemon_cmdln_opt {
	int                     short_name;
	const char *            long_name;
	const char *            arg_name;
	bool                    required;
	const char *            help;
	srplug_cmdln_parse_fn * parse;
};

#define SRPLUG_OPT_MAX (1 << 23)

struct srplug_daemon_cmdln {
	const char *                     brief;
	unsigned int                     nr;
	struct srplug_daemon_cmdln_opt * opts;
};

extern int
srplug_daemon_cmdln_parse(int                                argc,
                          char *                             argv[],
                          const struct srplug_daemon_cmdln * cmdln,
                          struct srplug_daemon_conf *        config)
	__srplug_nonull(2, 3, 4) __warn_result __srplug_export;

/******************************************************************************
 * Logging handling.
 ******************************************************************************/

extern struct elog *
srplug_daemon_create_log(const struct srplug_daemon_conf * config)
	__srplug_export;

#if defined(CONFIG_SRPLUG_LOG)

extern void
srplug_daemon_destroy_log(struct elog * logger)
	__srplug_export;

#else  /* !defined(CONFIG_SRPLUG_LOG) */

static inline void
srplug_daemon_destroy_log(struct elog * logger __unused)
{
}

#endif /* defined(CONFIG_SRPLUG_LOG) */

/******************************************************************************
 * Main daemon handling.
 ******************************************************************************/

struct srplug_sigs_work {
	struct upoll_worker base;
	int                 fd;
};

struct srplug_daemon {
	sr_session_ctx_t *      sess;
	struct upoll            poll;
	struct upoll_worker     sub_work;
	sr_subscription_ctx_t * sub_ctx;
	struct etux_timer       sub_tmr;
	unsigned int            sub_cnt;
	struct srplug_sigs_work sigs;
};

#define srplug_daemon_assert(_daemon) \
	srplug_assert(_daemon); \
	srplug_assert((_daemon)->sess)

static inline __srplug_nonull(1) __returns_nonull
sr_session_ctx_t *
srplug_daemon_session(const struct srplug_daemon * daemon)
{
	srplug_daemon_assert(daemon);

	return daemon->sess;
}

static inline __srplug_nonull(1) __returns_nonull
const struct upoll *
srplug_daemon_poller(const struct srplug_daemon * daemon)
{
	srplug_daemon_assert(daemon);

	return (const struct upoll *)&daemon->poll;
}

extern int
srplug_daemon_change_subscribe(struct srplug_daemon *           daemon,
                               const struct srplug_change_sub * subscription,
                               void *                           data)
	__srplug_nonull(1, 2) __warn_result __srplug_export;

extern int
srplug_daemon_oper_subscribe(struct srplug_daemon *         daemon,
                             const struct srplug_oper_sub * subscription,
                             void *                         data)
	__srplug_nonull(1, 2) __warn_result __srplug_export;

extern int
srplug_daemon_rpc_subscribe(struct srplug_daemon *        daemon,
                            const struct srplug_rpc_sub * subscription,
                            void *                        data)
	__srplug_nonull(1, 2) __warn_result __srplug_export;

extern int
srplug_daemon_subscribe(struct srplug_daemon *    daemon,
                        const struct srplug_sub * subscription,
                        void *                    data)
	__srplug_nonull(1, 2) __warn_result __srplug_export;

extern int
srplug_daemon_subscribe_all(struct srplug_daemon *    daemon,
                            const struct srplug_sub * subscriptions,
                            unsigned int              nr,
                            void *                    data)
	__srplug_nonull(1, 2) __warn_result __srplug_export;

extern int
srplug_daemon_poll(const struct srplug_daemon * daemon)
	__srplug_nonull(1) __warn_result __srplug_export;

extern int
srplug_daemon_open(struct srplug_daemon * daemon, unsigned int poll_nr)
	__srplug_nonull(1) __warn_result __srplug_export;

extern void
srplug_daemon_close(struct srplug_daemon * daemon)
	__srplug_nonull(1) __srplug_export;

#endif /* defined(CONFIG_SRPLUG_DAEMON) */
