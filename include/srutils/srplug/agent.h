#ifndef _SRPLUG_AGENT_H
#define _SRPLUG_AGENT_H

#include <srutils/srplug/common.h>

struct srplug_agent {
	struct srplug_object base;
	const struct upoll * poll;
};

#define srplug_agent_assert(_agent) \
	srplug_assert(_agent); \
	srplug_object_assert(&(_agent)->base); \
	srplug_assert((_agent)->poll)

static inline __srplug_nonull(1) __returns_nonull
sr_session_ctx_t *
srplug_agent_session(const struct srplug_agent * agent)
{
	srplug_agent_assert(agent);

	return srplug_object_session(&agent->base);
}

static inline __srplug_nonull(1) __returns_nonull
struct upoll_worker *
srplug_agent_worker(const struct srplug_agent * agent)
{
	srplug_agent_assert(agent);

	return srplug_object_worker(&agent->base);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_change_subscribe(struct srplug_agent *            agent,
                              const struct srplug_change_sub * subscription,
                              void *                           data)
{
	srplug_agent_assert(agent);
	srplug_assert_change_sub(subscription);

	return srplug_object_change_subscribe(&agent->base,
	                                      subscription,
	                                      data,
	                                      agent->poll);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_oper_subscribe(struct srplug_agent *          agent,
                            const struct srplug_oper_sub * subscription,
                            void *                         data)
{
	srplug_agent_assert(agent);
	srplug_assert_oper_sub(subscription);

	return srplug_object_oper_subscribe(&agent->base,
	                                    subscription,
	                                    data,
	                                    agent->poll);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_rpc_subscribe(struct srplug_agent *         agent,
                           const struct srplug_rpc_sub * subscription,
                           void *                        data)
{
	srplug_agent_assert(agent);
	srplug_assert_rpc_sub(subscription);

	return srplug_object_rpc_subscribe(&agent->base,
	                                   subscription,
	                                   data,
	                                   agent->poll);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_subscribe(struct srplug_agent *     agent,
                       const struct srplug_sub * subscription,
                       void *                    data)
{
	srplug_agent_assert(agent);
	srplug_assert(subscription);

	return srplug_object_subscribe(&agent->base,
	                               subscription,
	                               data,
	                               agent->poll);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_subscribe_all(struct srplug_agent *     agent,
                           const struct srplug_sub * subscriptions,
                           unsigned int              nr,
                           void *                    data)
{
	srplug_agent_assert(agent);
	srplug_assert(subscriptions);
	srplug_assert(nr);

	return srplug_object_subscribe_all(&agent->base,
	                                   subscriptions,
	                                   nr,
	                                   data,
	                                   agent->poll);
}

static inline __srplug_nonull(1, 2) __warn_result
int
srplug_agent_open(struct srplug_agent * agent, const struct upoll * poller)
{
	srplug_assert(agent);
	srplug_assert(poller);

	int err;

	err = srplug_object_open(&agent->base);
	if (err)
		return err;

	agent->poll = poller;

	return 0;
}

static inline __srplug_nonull(1)
void
srplug_agent_close(struct srplug_agent * agent)
{
	srplug_agent_assert(agent);

	srplug_object_close(&agent->base, agent->poll);
}

#endif /* _SRPLUG_AGENT_H */
