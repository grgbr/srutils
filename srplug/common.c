#include "log.h"
#include "srutils/srepo/data.h"
#include "srutils/srepo/schema.h"

sr_error_t
srplug_probe_feature(const struct ly_ctx * context,
                     const char *          module,
                     struct srplug_feat *  feature)
{
	srplug_assert(context);
	srplug_assert(module);
	srplug_assert(module[0]);
	srplug_assert(feature);
	srplug_assert(feature->name);
	srplug_assert(feature->name[0]);

	const struct lys_module * mod;
	LY_ERR                    ret;

	mod = srepo_sch_find_module(context, module);
	if (!mod) {
		srplug_warn("'%s' module: not found", mod->name);
		return SR_ERR_NOT_FOUND;
	}

	ret = lys_feature_value(mod, feature->name);
	switch (ret) {
	case LY_SUCCESS:
		feature->on = true;
		return SR_ERR_OK;

	case LY_ENOT:
		feature->on = false;
		return SR_ERR_OK;

	case LY_ENOTFOUND:
		break;

	default:
		srplug_assert(0);
	}

	srplug_warn("'%s' module: '%s' feature: not found",
	            mod->name,
	            feature->name);

	return SR_ERR_NOT_FOUND;
}

const struct lys_module *
srplug_find_module(const struct ly_ctx * context, const char * module)
{
	srplug_assert(context);
	srplug_assert(module);
	srplug_assert(module[0]);

	const struct lys_module * mod;

	mod = srepo_sch_find_module(context, module);
	if (mod)
		return mod;

	srplug_warn("'%s' module: not found", module);

	return NULL;
}

#if defined(CONFIG_SRPLUG_DEBUG)

static
const char *
srplug_change_oper_str(sr_change_oper_t oper)
{
	switch (oper) {
	case SR_OP_CREATED:
		return "created";
	case SR_OP_MODIFIED:
		return "modified";
	case SR_OP_DELETED:
		return "deleted";
	case SR_OP_MOVED:
		return "moved";
	default:
		break;
	}

	return "unknown";
}

sr_error_t
srplug_change_debug(const sr_session_ctx_t * session,
                    const struct lyd_node *  node,
                    sr_change_oper_t         oper,
                    const char *             old,
                    void *                   data)
{
	srplug_assert(session);
	srplug_assert(node);

	srplug_sess_node_debug(session,
	                       node,
	                       "%s change event: %s%s%s --> '%s' [data:%p]",
	                       srplug_change_oper_str(oper),
	                       old ? "'" : "",
	                       old ? old : "none",
	                       old ? "'" : "",
	                       srepo_dat_node_as_str(node),
	                       data);
	return SR_ERR_OK;
}

#endif /* defined(CONFIG_SRPLUG_DEBUG) */

sr_error_t
srplug_handle_changes(sr_session_ctx_t *        session,
                      const char *              xpath,
                      srplug_handle_change_fn * handle,
                      void *                    data)
{
	srplug_assert(session);
	srplug_assert(xpath);
	srplug_assert(handle);

	sr_change_iter_t * iter;
	sr_error_t         ret;

	if (srepo_xpath_validate(xpath) < 0) {
		srplug_assert(0);
		return SR_ERR_INTERNAL;
	}

	ret = sr_get_changes_iter(session, xpath, &iter);
	srplug_assert(ret != SR_ERR_INVAL_ARG);
	if (ret == SR_ERR_NO_MEMORY)
		srepo_abort();
	else if (ret != SR_ERR_OK)
		return ret;

	do {
		sr_change_oper_t        oper;
		const struct lyd_node * node;
		const char *            old;

		ret = sr_get_change_tree_next(session,
		                              iter,
		                              &oper,
		                              &node,
		                              &old,
		                              NULL,
		                              NULL);
		srplug_assert(ret != SR_ERR_INVAL_ARG);
		if (ret == SR_ERR_NO_MEMORY)
			srepo_abort();
		if (ret != SR_ERR_OK)
			break;

		ret = handle(session, node, oper, old, data);
	} while (ret == SR_ERR_OK);

	sr_free_change_iter(iter);

	return (ret == SR_ERR_NOT_FOUND) ? SR_ERR_OK : ret;
}

/*
 * Sysrepo pluging data change dispatcher.
 */
struct srplug_change_dispatch {
	unsigned int                       nr;
	const struct srplug_change_hndlr * hndlrs;
	void *                             data;
};

#define srplug_assert_change_dispatch(_dispatch) \
	srplug_assert(_dispatch); \
	srplug_assert((_dispatch)->nr); \
	srplug_assert((_dispatch)->hndlrs)

#define srplug_assert_change_hndlr(_hndlr) \
	srplug_assert(_hndlr); \
	srplug_assert(srepo_xpath_validate_node((_hndlr)->name)); \
	srplug_assert((_hndlr)->handle)

static sr_error_t
srplug_dispatch_child_change(sr_session_ctx_t *      session,
                             const struct lyd_node * node,
                             sr_change_oper_t        oper,
                             const char *            old,
                             void *                  dispatch)
{
	srplug_assert_change_dispatch((const struct srplug_change_dispatch *)
	                              dispatch);

	const char *                          name;
	unsigned int                          h;
	const struct srplug_change_dispatch * disp = dispatch;

	name = srepo_dat_node_name(node);
	if (!name) {
		srplug_sess_node_warn(session, node, "invalid node name");
		return SR_ERR_INVAL_ARG;
	}

	for (h = 0; h < disp->nr; h++) {
		const struct srplug_change_hndlr * hndlr = &disp->hndlrs[h];

		srplug_assert_change_hndlr(hndlr);

		if ((!hndlr->feature || hndlr->feature->on) &&
		    !strcmp(name, hndlr->name)) {
			srplug_sess_node_debug(session,
			                       node,
			                       "handling '%s' change operation",
			                       srplug_change_oper_str(oper));
			return hndlr->handle(session,
			                     node,
			                     oper,
			                     old,
			                     disp->data);
		}
	}

	srplug_sess_node_debug(session,
	                       node,
	                       "no change handler found: ignoring");

	return SR_ERR_OK;
}

sr_error_t
srplug_process_child_changes(sr_session_ctx_t *                 session,
                             const char *                       xpath,
                             const struct srplug_change_hndlr * handlers,
                             unsigned int                       nr,
                             void *                             data)
{
	srplug_assert(session);
	srplug_assert(srepo_xpath_validate(xpath) > 0);
	srplug_assert(handlers);
	srplug_assert(nr);

	char *                              pth;
	int                                 ret;
	const struct srplug_change_dispatch disp = {
		.nr     = nr,
		.hndlrs = handlers,
		.data   = data
	};

	ret = srepo_xpath_createf(&pth, "%s/*", xpath);
	srplug_assert(ret);
	if (ret < 0)
		return srepo_sys_error(ret);

	ret = srplug_handle_changes(session,
	                            pth,
	                            srplug_dispatch_child_change,
	                            (void *)&disp);
	srepo_free(pth);

	return ret;
}

static int
srplug_register_change_sub(sr_subscription_ctx_t **         subscriptions,
                           sr_session_ctx_t *               session,
                           const struct srplug_change_sub * subscription,
                           void *                           data)
{
	if (!subscription->feature || subscription->feature->on) {
		sr_error_t err;

		err = sr_module_change_subscribe(
			session,
			subscription->module,
			subscription->xpath,
			subscription->on_change,
			data,
			subscription->priority,
			subscription->options | SR_SUBSCR_NO_THREAD,
			subscriptions);
		if (err == SR_ERR_OK) {
			srplug_sess_debug(
				session,
				"'%s%s': configuration data handler registered",
				subscription->xpath ? "" : "/",
				subscription->xpath ? subscription->xpath
				                    : subscription->module);
			return 0;
		}

		if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		srplug_sess_info(
			session,
			"'%s%s': cannot register configuration data handler: %s",
			subscription->xpath ? "" : "/",
			subscription->xpath ? subscription->xpath
			                    : subscription->module,
			srepo_errstr(err));

		return -EPERM;
	}
	else
		return 0;
}

static int
srplug_register_oper_sub(sr_subscription_ctx_t **       subscriptions,
                         sr_session_ctx_t *             session,
                         const struct srplug_oper_sub * subscription,
                         void *                         data)
{
	if (!subscription->feature || subscription->feature->on) {
		int err;

		err = sr_oper_get_subscribe(
			session,
			subscription->module,
			subscription->xpath,
			subscription->on_get,
			data,
			subscription->options | SR_SUBSCR_NO_THREAD,
			subscriptions);
		if (err == SR_ERR_OK) {
			srplug_sess_debug(
				session,
				"'%s%s': operational data handler registered",
				subscription->xpath ? "" : "/",
				subscription->xpath ? subscription->xpath
				                    : subscription->module);
			return 0;
		}


		if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		srplug_sess_info(
			session,
			"'%s%s': cannot register operational data handler: %s",
			subscription->xpath ? "" : "/",
			subscription->xpath ? subscription->xpath
			                    : subscription->module,
			srepo_errstr(err));

		return -EPERM;
	}
	else
		return 0;
}

static int
srplug_register_rpc_sub(sr_subscription_ctx_t **      subscriptions,
                        sr_session_ctx_t *            session,
                        const struct srplug_rpc_sub * subscription,
                        void *                        data)
{
	if (!subscription->feature || subscription->feature->on) {
		int err;

		err = sr_rpc_subscribe(
			session,
			subscription->xpath,
			subscription->on_rpc,
			data,
			subscription->priority,
			subscription->options | SR_SUBSCR_NO_THREAD,
			subscriptions);
		if (err == SR_ERR_OK) {
			srplug_sess_debug(
				session,
				"'%s': RPC / action handler registered",
				subscription->xpath ? subscription->xpath : "");
			return 0;
		}

		if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		srplug_sess_info(
			session,
			"'%s': cannot register RPC / action handler: %s",
			subscription->xpath ? subscription->xpath : "",
			srepo_errstr(err));

		return -EPERM;
	}
	else
		return 0;
}

static void
srplug_adjust_tmr(struct etux_timer * timer, const struct timespec * tmout)
{
	if (tmout->tv_sec || tmout->tv_nsec) {
		struct timespec tm;

		/* Compute requested timeout absolute time. */
		utime_monotonic_now(&tm);
		utime_tspec_add_clamp(&tm, tmout);

		/*
		 * And arm the timer if necessary, i.e., if it is not armed or
		 * if its expiry date is later than the requested one.
		 */
		if (!etux_timer_is_armed(timer) ||
		    utime_tspec_after(etux_timer_expiry_tspec(timer), &tm))
			etux_timer_arm_tspec(timer, &tm);
	}
	else
		etux_timer_cancel(timer);
}

static
void
srplug_process_subs(sr_subscription_ctx_t * subscriptions,
                    sr_session_ctx_t *      session,
                    struct etux_timer *     timer)
{
	struct timespec tmout;
	int             err;

	/*
	 * No need to initialize `tmout' since sr_subscription_process_events()
	 * will zero it out.
	 */
	err = sr_subscription_process_events(subscriptions, session, &tmout);
	switch (err) {
	case SR_ERR_OK:
		break;

	case SR_ERR_TIME_OUT:
		/*
		 * Reschedule a call to sr_subscription_process_events() no
		 * later than 100 milliseconds in the future.
		 */
		tmout.tv_sec = 0;
		tmout.tv_nsec = 100000000;
		break;

	case SR_ERR_NO_MEMORY: /* Not enough memory. */
		srepo_abort();

	default:
		srplug_err("failed to process subscription events: %s",
		           srepo_errstr(err));
		return;
	}

	/* Adjust timer configuration. */
	srplug_adjust_tmr(timer, &tmout);
}

static void
srplug_clear_subs(sr_subscription_ctx_t *  subscriptions,
                  const sr_session_ctx_t * session __unused)
{
	sr_error_t err;

	/*
	 * Free all subscribtions (worker->ctx may be NULL here).
	 *
	 * Warning:
	 * Although all sysrepo code samples don't check the code returned by
	 * calls to sr_unsubscribe(), there might be an error here (because of
	 * locking timeouts and other things).
	 * What should we do here ? Can this really happen ?
	 * (Do not forget our use case is single threaded and relies upon an
	 * asynchronous event poller logic).
	 */
	err = sr_unsubscribe(subscriptions);
	if (err != SR_ERR_OK) {
		if (err == SR_ERR_NO_MEMORY)
			srepo_abort();

		srplug_sess_warn(session,
		                 "cannot clear subscriptions: %s",
		                 srepo_errstr(err));
	}
}

static
void
srplug_object_expire_subs(struct etux_timer * timer)
{
	const struct srplug_object * object;

	object = containerof(timer, struct srplug_object, tmr);
	srplug_object_assert(object);

	srplug_process_subs(object->ctx, object->sess, timer);
}

static
int
srplug_object_dispatch_subs(struct upoll_worker * worker,
                           uint32_t              state __unused,
                           const struct upoll *  poller __unused)
{
	struct srplug_object * object;

	object = containerof(worker, struct srplug_object, work);
	srplug_object_assert(object);

	srplug_process_subs(object->ctx, object->sess, &object->tmr);

	return 0;
}

static int
srplug_object_enable_subs(struct srplug_object * object,
                          const struct upoll *  poller)
{
	int err;

	if (!object->cnt) {
		srplug_assert(object->ctx);
		srplug_assert(!etux_timer_is_armed(&object->tmr));

		int fd;

		err = sr_get_event_pipe(object->ctx, &fd);
		srplug_assert(err == SR_ERR_OK);
		srplug_assert(fd >= 0);

		err = upoll_register_dispatch(poller,
		                              fd,
		                              EPOLLIN,
		                              &object->work,
		                              srplug_object_dispatch_subs);
		if (err) {
			if (err == -ENOMEM)
				srepo_abort();

			srplug_clear_subs(object->ctx, object->sess);
			object->ctx = NULL;

			srplug_err("cannot enable subscription worker: %s",
			           strerror(-err));

			return err;
		}

		srplug_sess_debug(object->sess, "subscription worker enabled");
	}

	object->cnt++;

	return 0;
}

static void
srplug_object_disable_subs(struct srplug_object * object,
                           const struct upoll *  poller)
{
	if (object->cnt) {
		srplug_assert(object->work.dispatch);
		srplug_assert(object->ctx);

		int fd;
		int err __unused;

		etux_timer_cancel(&object->tmr);

		err = sr_get_event_pipe(object->ctx, &fd);
		srplug_assert(err == SR_ERR_OK);
		srplug_assert(fd >= 0);

		/* Unregister from asynchronous poller. */
		upoll_unregister(poller, fd);

		object->cnt = 0;

		srplug_sess_debug(object->sess, "subscription worker disabled");
	}
}

int
srplug_object_change_subscribe(struct srplug_object *           object,
                               const struct srplug_change_sub * subscription,
                               void *                           data,
                               const struct upoll *             poller)
{
	srplug_object_assert(object);
	srplug_assert_change_sub(subscription);
	srplug_assert(poller);

	int err;

	/* Register the configuration data subscription handler. */
	err = srplug_register_change_sub(&object->ctx,
	                                 object->sess,
	                                 subscription,
	                                 data);
	if (err)
		return err;

	/* Enable asynchronous processing of subscriptions. */
	return srplug_object_enable_subs(object, poller);
}

int
srplug_object_oper_subscribe(struct srplug_object *         object,
                             const struct srplug_oper_sub * subscription,
                             void *                         data,
                             const struct upoll *           poller)
{
	srplug_object_assert(object);
	srplug_assert_oper_sub(subscription);
	srplug_assert(poller);

	int err;

	/* Register the operational data subscription handler. */
	err = srplug_register_oper_sub(&object->ctx,
	                               object->sess,
	                               subscription,
	                               data);
	if (err)
		return err;

	/* Enable asynchronous processing of subscriptions. */
	return srplug_object_enable_subs(object, poller);
}

int
srplug_object_rpc_subscribe(struct srplug_object *        object,
                            const struct srplug_rpc_sub * subscription,
                            void *                        data,
                            const struct upoll *          poller)
{
	srplug_object_assert(object);
	srplug_assert_rpc_sub(subscription);
	srplug_assert(poller);

	int err;

	/* Register the RPC subscription handler. */
	err = srplug_register_rpc_sub(&object->ctx,
	                              object->sess,
	                              subscription,
	                              data);
	if (err)
		return err;

	/* Enable asynchronous processing of subscriptions. */
	return srplug_object_enable_subs(object, poller);
}

int
srplug_object_subscribe(struct srplug_object *    object,
                        const struct srplug_sub * subscription,
                        void *                    data,
                        const struct upoll *      poller)
{
	srplug_object_assert(object);
	srplug_assert(subscription);
	srplug_assert(subscription->kind >= 0);
	srplug_assert(subscription->kind < SRPLUG_SUB_KIND_NR);
	srplug_assert(poller);

	switch (subscription->kind) {
	case SRPLUG_CHANGE_SUB_KIND:
		return srplug_object_change_subscribe(object,
		                                      &subscription->change,
		                                      data,
		                                      poller);

	case SRPLUG_OPER_SUB_KIND:
		return srplug_object_oper_subscribe(object,
		                                    &subscription->oper,
		                                    data,
		                                    poller);

	case SRPLUG_RPC_SUB_KIND:
		return srplug_object_rpc_subscribe(object,
		                                   &subscription->rpc,
		                                   data,
		                                   poller);

	default:
		srplug_assert(0);
		return -ENOTSUP;
	}
}

int
srplug_object_subscribe_all(struct srplug_object *    object,
                            const struct srplug_sub * subscriptions,
                            unsigned int              nr,
                            void *                    data,
                            const struct upoll *      poller)
{
	srplug_object_assert(object);
	srplug_assert(subscriptions);
	srplug_assert(nr);
	srplug_assert(poller);

	unsigned int             s;
	sr_subscription_ctx_t ** ctx = &object->ctx;
	sr_session_ctx_t *       sess = object->sess;

	for (s = 0; s < nr; s++) {
		srplug_assert(subscriptions[s].kind >= 0);
		srplug_assert(subscriptions[s].kind < SRPLUG_SUB_KIND_NR);

		const struct srplug_sub * sub = &subscriptions[s];
		int                       err;

		switch (sub->kind) {
		case SRPLUG_CHANGE_SUB_KIND:
			err = srplug_register_change_sub(ctx,
			                                 sess,
			                                 &sub->change,
			                                 data);
			break;

		case SRPLUG_OPER_SUB_KIND:
			err = srplug_register_oper_sub(ctx,
			                               sess,
			                               &sub->oper,
			                               data);
			break;

		case SRPLUG_RPC_SUB_KIND:
			err = srplug_register_rpc_sub(ctx,
			                              sess,
			                              &sub->rpc,
			                              data);
			break;

		default:
			srplug_assert(0);
			err = -ENOTSUP;
		}

		if (err)
			return err;
	}

	/* Enable asynchronous processing of subscriptions. */
	return srplug_object_enable_subs(object, poller);
}

int
srplug_object_open(struct srplug_object * object)
{
	srplug_assert(object);

	sr_error_t         err;
	sr_conn_ctx_t *    conn = NULL;
	sr_session_ctx_t * sess = NULL;

	/* Connect to sysrepo */
	err = srepo_connect(&conn);
	if (err != SR_ERR_OK) {
		srplug_err("cannot connect to datastore: %s",
		           srepo_errstr(err));
		return -EPERM;
	}

	err = srepo_session_start(conn, SR_DS_RUNNING, &sess);
	if (err) {
		srplug_err("cannot start datastore session: %s",
		           srepo_errstr(err));
		goto disconnect;
	}

	object->sess = sess;
	object->ctx = NULL;
	etux_timer_init(&object->tmr, srplug_object_expire_subs);
	object->cnt = 0;

	srplug_sess_debug(object->sess, "sysrepo object opened");

	return 0;

disconnect:
	/* Also closes all sessions related to this connection. */
	srepo_disconnect(conn);

	return -EPERM;
}

void
srplug_object_close(struct srplug_object * object, const struct upoll *  poller)
{
	srplug_object_assert(object);

	sr_conn_ctx_t * conn = sr_session_get_connection(object->sess);

	srplug_sess_debug(object->sess, "closing sysrepo object.");

	/* Stop asynchronous processing of subscriptions. */
	srplug_object_disable_subs(object, poller);

#if 0
	/*
	 * Release all subscriptions.
	 * Keep this code for reference purpose only since all subscriptions are
	 * free'd at srepo_disconnect() time.
	 */
	srplug_assert(!etux_timer_is_armed(&object->tmr));
	srplug_clear_subs(object->ctx, object->sess);

	/*
	 * Close Sysrepo datastore session.
	 * Keep this code for reference purpose only since all sessions are
	 * free'd at srepo_disconnect() time.
	 */
	srepo_session_stop(object->sess);
#endif

	/* Close all sessions and connection to Sysrepo datastores. */
	srepo_disconnect(conn);
}
