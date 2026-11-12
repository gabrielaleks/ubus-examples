SDK    = /home/builder/sdk/staging_dir
TARGET = $(SDK)/target-aarch64_cortex-a53_musl
CROSS  = CC=aarch64-openwrt-linux-musl-gcc \
         CFLAGS="-Wall -O2 -I$(TARGET)/usr/include" \
         LDFLAGS="-L$(TARGET)/usr/lib"
DOCKER = docker run --rm --platform linux/amd64 -v "$(CURDIR)":/work -w /work \
         -e STAGING_DIR=$(SDK) \
         -e PATH=$(SDK)/toolchain-aarch64_cortex-a53_gcc-14.3.0_musl/bin:/usr/bin:/bin \
         ubus-sdk

EXAMPLES = request-response
BPI     ?= root@192.168.20.1

build:
	$(DOCKER) sh -c 'for d in $(EXAMPLES); do make -C $$d $(CROSS) || exit 1; done'

deploy: build
	scp -O request-response/server $(BPI):/tmp/
	scp -O request-response/client $(BPI):/tmp/

clean:
	for d in $(EXAMPLES); do $(MAKE) -C $$d clean; done

.PHONY: build deploy clean