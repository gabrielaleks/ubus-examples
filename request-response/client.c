#include <libubus.h>
#include <stdio.h>
#include <stdlib.h>

static struct blob_buf b;

enum {
    VALUE,
    __VALUE_MAX
};

static const struct blobmsg_policy value_policy[__VALUE_MAX] = {
    [VALUE] = {.name = "value", .type = BLOBMSG_TYPE_INT32},
};

// Runs once for the data part of the reply
static void reply_cb(
    struct ubus_request* req,
    int type,
    struct blob_attr* msg
) {
    uint32_t* out = req->priv;
    struct blob_attr* tb[__VALUE_MAX];

    blobmsg_parse(
        value_policy,
        __VALUE_MAX,
        tb,
        blob_data(msg),
        blob_len(msg)
    );

    if (tb[VALUE]) {
        *out = blobmsg_get_u32(tb[VALUE]);
    }
}

int main(int argc, char** argv) {
    struct ubus_context* ctx;
    uint32_t id;
    int ret;

    ctx = ubus_connect(NULL);

    if (!ctx) {
        fprintf(stderr, "cannot connect to ubusd\n");
        return 1;
    }

    if (ubus_lookup_id(ctx, "demo.counter", &id)) {
        fprintf(stderr, "demo.counter not found\n");
        ubus_free(ctx);
        return 1;
    }

    blob_buf_init(&b, 0);

    uint32_t value = 0;
    if (argc > 1) {  // client 5  -> add {"n": 5}
        blobmsg_add_u32(&b, "n", atoi(argv[1]));
        ret = ubus_invoke(ctx, id, "add", b.head, reply_cb, &value, 3000);
    } else {  // client -> get
        ret = ubus_invoke(ctx, id, "get", b.head, reply_cb, &value, 3000);
    }

    if (ret) {
        fprintf(stderr, "call failed: %s\n", ubus_strerror(ret));
    } else {
        printf("counter = %u\n", value);
    }

    ubus_free(ctx);
    return ret;
}
