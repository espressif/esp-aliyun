#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    user \
            components/aliyun/platform

endif # } PDIR

LDDIR = ./ld

CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

LD_FILE = $(LDDIR)/eagle.app.v6.ld

ifneq ($(app),0)
    ifneq ($(findstring $(size_map),  6  8  9),)
      LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
    else
      ifeq ($(size_map), 5)
        LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).2048.ld
      else
        ifeq ($(size_map), 4)
          LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
        else
          ifeq ($(size_map), 3)
            LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
          else
            ifeq ($(size_map), 2)
              LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).1024.app$(app).ld
            else
              ifeq ($(size_map), 0)
                LD_FILE = $(LDDIR)/eagle.app.v6.$(boot).512.app$(app).ld
              endif
            endif
          endif
        endif
      endif
    endif
endif

# disable compile submodule repeatedly
dummy: all

ALIYUN_PATH = components/aliyun/iotkit-embedded

aliyun:
	cp $(ALIYUN_PATH)/src/configs/default_settings.mk $(ALIYUN_PATH)/src/configs/default_settings.mk.bak
	cp components/aliyun/config/* $(ALIYUN_PATH)/src/configs/
	make -C $(ALIYUN_PATH) distclean
	make -C $(ALIYUN_PATH)
	rm $(ALIYUN_PATH)/src/configs/config.espressif.esp8266
	mv $(ALIYUN_PATH)/src/configs/default_settings.mk.bak $(ALIYUN_PATH)/src/configs/default_settings.mk

COMPONENTS_eagle.app.v6 = \
    user/libuser.a \
    components/aliyun/platform/libplatform.a

LINKFLAGS_eagle.app.v6 = \
	-L$(SDK_PATH)/lib        \
	-Wl,--gc-sections   \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lcirom \
	-lgcc					\
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-lwpa	\
	-lmain	\
	-lfreertos	\
	-llwip	\
	-lssl	\
	-ldriver \
	-lmirom\
	-lmbedtls               \
    -lopenssl               \
	-ljson  \
	-lsmartconfig \
	-lspiffs	\
	$(DEP_LIBS_eagle.app.v6)					\
	-lcrypto \
	-Wl,--end-group
	
DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH -DMQTT_DIRECT

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

#PDIR := ../$(PDIR)

INCLUDES := $(INCLUDES) -I $(PDIR)include -I include -I $(SDK_PATH)/include/openssl
INCLUDES += -I $(PDIR)platforms/aliyun/platform/include
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/sdk-impl/include
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/packages/LITE-utils
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/packages/LITE-log
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/utils/digest
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/packages/iot-coap-c
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/sdk-impl/imports
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/utils/misc
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/system
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/import/linux/include
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/guider
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/security
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/sdk-impl
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/packages/LITE-log
INCLUDES += -I $(PDIR)platforms/aliyun/IoT-SDK_V2.0/src/mqtt
INCLUDES += -I $(PDIR)include/ota
INCLUDES += -I $(PDIR)include/mqtt

sinclude $(SDK_PATH)/Makefile

.PHONY: FORCE
FORCE:

