FROM debian:bookworm

# Tools the SDK needs on the build machine
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential ca-certificates file gawk gettext git libncurses-dev \
    python3 python3-distutils rsync unzip wget zstd \
&& rm -rf /var/lib/apt/lists/*

# The OpenWrt build refuses to run as root, so use a normal user
RUN useradd -m builder
USER builder
WORKDIR /home/builder

# The SDK for the router's image: release 25.12.6, target mediatek/filogic.
# Change these three values for another router (see /etc/openwrt_release).
ARG SDK_BASE=https://downloads.openwrt.org/releases/25.12.6/targets/mediatek/filogic
ARG SDK_FILE=openwrt-sdk-25.12.6-mediatek-filogic_gcc-14.3.0_musl.Linux-x86_64.tar.zst
ARG SDK_SHA256=1c04ac47bdfc361d6aab299eb520d570e368013dedcb8d29cf39dc7168ae642e

# Download, check that the file is the one OpenWrt published, unpack
RUN wget -q "$SDK_BASE/$SDK_FILE" \
 && echo "$SDK_SHA256  $SDK_FILE" | sha256sum -c - \
 && mkdir sdk && tar --zstd -xf "$SDK_FILE" -C sdk --strip-components=1 \
 && rm "$SDK_FILE"

WORKDIR /home/builder/sdk

# Get the recipes for libubox and ubus, and create the build config
RUN ./scripts/feeds update base \
&& ./scripts/feeds install libubox ubus \
&& make defconfig \
&& make package/libubox/compile package/ubus/compile