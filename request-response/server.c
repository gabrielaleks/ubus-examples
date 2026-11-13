#include <libubus.h>
#include <stdint.h>

static struct blob_buf b;
static uint32_t counter;

enum {
    ADD_N,
    __ADD_MAX
};

static const struct blobmsg_policy add_policy[__ADD_MAX] = {
    [ADD_N] = {.name = "n", .type = BLOBMSG_TYPE_INT32},
};

static int counter_get(
    struct ubus_context* ctx,
    struct ubus_object* obj,
    struct ubus_request_data* req,
    const char* method,
    struct blob_attr* msg
) {
    blob_buf_init(&b, 0);
    blobmsg_add_u32(&b, "value", counter);
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}

static int counter_add(
    struct ubus_context* ctx,
    struct ubus_object* obj,
    struct ubus_request_data* req,
    const char* method,
    struct blob_attr* msg
) {
    struct blob_attr* tb[__ADD_MAX];

    blobmsg_parse(
        add_policy,
        __ADD_MAX,
        tb,
        blob_data(msg),
        blob_len(msg)
    );

    /* libubus doesn't check; we must */
    if (!tb[ADD_N]) {
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    counter += blobmsg_get_u32(tb[ADD_N]);
    return counter_get(ctx, obj, req, method, msg);
}

static const struct ubus_method counter_methods[] = {
    UBUS_METHOD_NOARG("get", counter_get),
    UBUS_METHOD("add", counter_add, add_policy),
};

static struct ubus_object_type counter_type = UBUS_OBJECT_TYPE(
    "demo.counter",
    counter_methods
);

static struct ubus_object counter_obj = {
    .name = "demo.counter",
    .type = &counter_type,
    .methods = counter_methods,
    .n_methods = ARRAY_SIZE(counter_methods),
};

int main(void) {
    uloop_init();
    struct ubus_context* ctx = ubus_connect(NULL);

    if (!ctx) {
        return 1;
    }

    ubus_add_uloop(ctx);

    if (ubus_add_object(ctx, &counter_obj)) {
        return 1;
    }

    uloop_run();
    ubus_free(ctx);
    uloop_done();
    return 0;
}