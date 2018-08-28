#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := main

export PROJ_DIR ?= $(PWD)/../..

export IDF_PATH ?= $(PROJ_DIR)/esp-idf

EXTRA_COMPONENT_DIRS += $(PROJ_DIR)/components

CONFIG_ENABLE_MQTT_CFLAGS:=y

include $(IDF_PATH)/make/project.mk
