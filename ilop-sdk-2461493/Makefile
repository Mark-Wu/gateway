all: lib/libilop-hal.so linkkit-example gateway serial_test

#CC=mipsel-openwrt-linux-gcc

CFLAGS=-Wall -g -O0 -Iinclude -Llib  -Isrc/hal -D_PLATFORM_IS_LINUX_
LDFLAGS=-lilop-tls -lilop-sdk -lilop-hal -lpthread -lrt -ldl

lib/libilop-hal.so: src/hal/base64.c  src/hal/cJSON.c  \
					src/hal/HAL_Crypt_Linux.c  src/hal/HAL_OS_linux.c \
					src/hal/HAL_TCP_linux.c  src/hal/HAL_TLS_mbedtls.c  \
					src/hal/HAL_UDP_linux.c  src/hal/kv.c \
					src/hal/config.c src/hal/config.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ -lilop-tls -luci

linkkit-example: src/example/linkkit-example.c src/example/light.c src/hal/cJSON.c
	$(CC) $(CFLAGS) -o $@ $^ -lilop-sdk -luci $(LDFLAGS)

gateway:	src/gateway/gateway_dev.c src/gateway/ble_subdev.c  \
			src/hal/cJSON.c src/gateway/serial_com.c \
			src/gateway/config.c src/gateway/config.h src/gateway/queue.c
	$(CC) $(CFLAGS) -o $@ $^ -lilop-sdk -luci $(LDFLAGS)

serial_test:	src/gateway/main.c	
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)
clean:
	rm lib/libilop-hal.so linkkit-example gateway
