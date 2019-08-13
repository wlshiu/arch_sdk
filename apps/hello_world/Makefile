#
# This is a project Makefile.
#

PROJECT_NAME := hello-world

BUILD_OUTPUT := out
ifeq ("$(origin O)", "command line")
  BUILD_OUTPUT := $(O)
endif

BUILD_OUTPUT := $(shell mkdir -p $(BUILD_OUTPUT) && cd $(BUILD_OUTPUT) && /bin/pwd)

include ../../tools/scripts/project.mk
# PROJECT_MK := $(wildcard ../../../*/project.mk)
# $(warning $(PROJECT_MK))

