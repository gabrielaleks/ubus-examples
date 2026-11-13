#include <libubus.h>
#include <stdio.h>

enum {
    VALUE,
    __VALUE_MAX
};

static const struct blobmsg_policy value_policy[__VALUE_MAX] = {
    [VALUE] = {.name = "value", .type = BLOBMSG_TYPE_INT32},
};

// Runs for each notification the object sends
static int on_notify(
    struct ubus_context* ctx,
    struct ubus_object* obj,
    struct ubus_request_data* req,
    const char* method,
    struct blob_attr* msg
) {
    struct blob_attr* tb[__VALUE_MAX];

    blobmsg_parse(
        value_policy,
        __VALUE_MAX,
        tb,
        blob_data(msg),
        blob_len(msg)
    );

    if (tb[VALUE]) {
        printf("%s: value = %u\n", method, blobmsg_get_u32(tb[VALUE]));
    }

    return UBUS_STATUS_OK;
}

// Runs when the object we follow goes away
static void on_remove(
    struct ubus_context* ctx,
    struct ubus_subscriber* sub,
    uint32_t id
) {
    printf("demo.counter is gone\n");
    uloop_end();
}

static struct ubus_subscriber subscriber = {
    .cb = on_notify,
    .remove_cb = on_remove,
};

int main(void) {
    struct ubus_context* ctx;
    uint32_t id;
    int ret;

    uloop_init();
    ctx = ubus_connect(NULL);

    if (!ctx) {
        fprintf(stderr, "cannot connect to ubusd\n");
        return 1;
    }

    ubus_add_uloop(ctx);

    ret = ubus_register_subscriber(ctx, &subscriber);
    if (ret) {
        fprintf(stderr, "cannot register subscriber: %s\n", ubus_strerror(ret));
        ubus_free(ctx);
        return 1;
    }

    if (ubus_lookup_id(ctx, "demo.counter", &id)) {
        fprintf(stderr, "demo.counter not found\n");
        ubus_free(ctx);
        return 1;
    }

    ret = ubus_subscribe(ctx, &subscriber, id);
    if (ret) {
        fprintf(stderr, "cannot subscribe: %s\n", ubus_strerror(ret));
        ubus_free(ctx);
        return 1;
    }

    uloop_run();

    ubus_free(ctx);
    uloop_done();
    return 0;
}
