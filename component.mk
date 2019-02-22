# Makefile

# CFLAGS +=

# version of iotkit-embedded: https://github.com/aliyun/iotkit-embedded
IE_VER := $(shell cd ${IE_PATH} && git describe --always --tags --dirty)

# version of esp-aliyun: https://github.com/espressif/esp-aliyun
EA_VER := $(shell cd ${ESP_ALIYUN_PATH} && git describe --always --tags --dirty)

CFLAGS += -DIE_VER=\"$(IE_VER)\"
CFLAGS += -DEA_VER=\"$(EA_VER)\"

COMPONENT_ADD_INCLUDEDIRS :=	\
iotkit-embedded/include			\
iotkit-embedded/include/imports		\
iotkit-embedded/include/exports		\
iotkit-embedded/src/services/awss	\
platform/crypto/include	\
platform/hal	\
platform/include

COMPONENT_SRCDIRS :=			\
platform/hal/os/espressif

ifdef CONFIG_TARGET_PLATFORM_ESP8266
# ESP8266 platform
COMPONENT_SRCDIRS += platform/crypto/src	\
platform/hal/ssl/openssl

else
# ESP32 platform
COMPONENT_SRCDIRS += platform/hal/ssl/mbedtls
endif

# link libiot_sdk.a
LIBS += iot_sdk
COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/iotkit-embedded/output/release/lib $(addprefix -l,$(LIBS))

ifdef CONFIG_TARGET_PLATFORM_ESP8266
LIBS += openssl
COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/platform/tls/library $(addprefix -l,$(LIBS))
endif