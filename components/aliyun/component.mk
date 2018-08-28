#
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS := \
    iotkit-embedded/src/sdk-impl \
    iotkit-embedded/src/sdk-impl/exports \
    iotkit-embedded/src/sdk-impl/imports \
    iotkit-embedded/src/packages/LITE-utils \
    iotkit-embedded/src/log/LITE-log

COMPONENT_SRCDIRS := \
    platform/os/espressif \
    platform/ssl/mbedtls 

# link libiot_sdk.a
LIBS += iot_sdk
COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/iotkit-embedded/output/release/lib $(addprefix -l,$(LIBS))
