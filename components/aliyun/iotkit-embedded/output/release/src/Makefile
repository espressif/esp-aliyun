.PHONY: all

CC          := gcc

CFLAGS      := -I../include
CFLAGS      += -I../include/iot-sdk
CFLAGS      += -I../include/mbedtls

LDFLAGS     := -L../lib
LDFLAGS     += -liot_sdk -liot_platform
LDFLAGS     += -Bstatic -liot_tls
LDFLAGS     += -lgcov

all ::
	$(CC) $(CFLAGS) -o ext.mqtt mqtt-example.c $(LDFLAGS)

clean ::
	rm -vf ext.mqtt

all ::
	$(CC) $(CFLAGS) -o ext.http http-example.c $(LDFLAGS)

clean ::
	rm -vf ext.http

