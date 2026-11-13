# ubus-examples

Small C programs that show how to use [ubus](https://openwrt.org/docs/techref/ubus), OpenWrt's micro bus, from code. There's one folder per pattern and each has both sides of the conversation:

| Folder | Pattern | Programs |
|---|---|---|
| `request-response/` | Calling methods on an object | `server` (registers `demo.counter`), `client` (calls `get` and `add`) |
| `events/` | Broadcasting by name | `sender` (the counter server, plus an event on every `add`), `listener` (prints `demo.*` events) |
| `subscriptions/` | Following one object | `publisher` (the counter server, plus a notification on every `add`), `subscriber` |

The code is explained step by step in the article [ubus: a guide to OpenWrt's micro bus](https://gabrielaleks.com/blog/ubus-a-guide-to-openwrts-micro-bus).

## Requirements

- An OpenWrt router. The examples are built for a Banana Pi BPI-R4 running OpenWrt 25.12.6 (`mediatek/filogic`, `aarch64_cortex-a53`). For another router, see [Using another router](#using-another-router).
- Docker on your computer. The build uses the official OpenWrt SDK, which only runs on x86_64 Linux, so it runs in a container. On Apple Silicon, turn on Docker Desktop's "Use Rosetta for x86_64/amd64 emulation" to make it faster.
- ssh/scp access to the router as root.

Nothing needs to be installed on the router: libubus, libubox and libblobmsg-json are part of every OpenWrt image.

## Build

Build the SDK image once. It downloads the SDK, checks its SHA256 and compiles libubox and ubus, so it takes a while:

```sh
docker build --platform linux/amd64 -t ubus-sdk .
```

Then build all the examples:

```sh
make
```

Each binary is written next to its source file (for example `request-response/server`). `make clean` removes them.

## Deploy

```sh
make deploy BPI=root@192.168.1.1
```

This builds, then copies all six binaries to `/tmp` on the router. Replace the address with your router's. `/tmp` is a RAM disk, so the binaries are gone after a reboot.

## Test

Open a few ssh sessions to the router. In each example, start the first program in one window and use the others from a second window.

Request/response:

```sh
/tmp/server                                  # window 1, keeps running
ubus call demo.counter add '{"n": 5}'        # window 2: {"value": 5}
/tmp/client                                  # counter = 5
/tmp/client 3                                # counter = 8
```

Events:

```sh
/tmp/sender                                  # window 1
/tmp/listener                                # window 2
/tmp/client 3                                # window 3; window 2 prints: event demo.counter.changed: {"value":3}
```

Subscriptions:

```sh
/tmp/publisher                               # window 1
/tmp/subscriber                              # window 2; window 1 prints: subscribers: yes
/tmp/client 6                                # window 3; window 2 prints: changed: value = 6
```

Stop each program with Ctrl-C. The article has more tests for each pattern, including the `ubus` CLI equivalents (`ubus listen`, `ubus send`, `ubus subscribe`).

## Using another router

Find your router's release, target and CPU:

```sh
cat /etc/openwrt_release     # DISTRIB_RELEASE, DISTRIB_TARGET, DISTRIB_ARCH
```

Then change:

1. `Dockerfile`: the three `SDK_*` values. They come from your release's download page, `https://downloads.openwrt.org/releases/<release>/targets/<target>/` (the SHA256 is in its `sha256sums` file).
2. `Makefile`: the `TARGET` and toolchain folder names, which contain your CPU and compiler version (`target-aarch64_cortex-a53_musl`, `toolchain-aarch64_cortex-a53_gcc-14.3.0_musl`). After building the image, list the right names with:

```sh
docker run --rm --platform linux/amd64 ubus-sdk ls staging_dir
```

Rebuild the image and run `make` again.

## Editor setup (optional)

To make `#include <libubus.h>` resolve in your editor, copy the SDK's headers out of the image:

```sh
docker run --rm --platform linux/amd64 -v "$PWD":/work ubus-sdk \
  sh -c 'cp -r staging_dir/target-*/usr/include /work/.sdk-include'
```

`.vscode/c_cpp_properties.json` already points VS Code's C/C++ extension at `.sdk-include`.

## License

[MIT](LICENSE)
