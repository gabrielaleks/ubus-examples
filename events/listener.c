#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <stdio.h>
#include <stdlib.h>

static void on_event(
    struct ubus_context* ctx,
    struct ubus_event_handler* ev,
    const char* type,
    struct blob_attr* msg
) {
    char* json = blobmsg_format_json(msg, true);
    printf("event %s: %s\n", type, json);
    free(json);
}

static struct ubus_event_handler listener = {.cb = on_event};

int main(void) {
    struct ubus_context* ctx;
    int ret;

    uloop_init();
    ctx = ubus_connect(NULL);

    if (!ctx) {
        fprintf(stderr, "cannot connect to ubusd\n");
        return 1;
    }

    ubus_add_uloop(ctx);

    ret = ubus_register_event_handler(ctx, &listener, "demo.*");
    if (ret) {
        fprintf(stderr, "cannot listen: %s\n", ubus_strerror(ret));
        ubus_free(ctx);
        return 1;
    }

    uloop_run();

    ubus_free(ctx);
    uloop_done();
    return 0;
}
