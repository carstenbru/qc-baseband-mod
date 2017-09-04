## @brief project Makefile
##
## This file contains project specific definitions,
## such as the used base firmware image
##
## @author Carsten Bruns (carst.bruns@gmx.de)

SEEMOO_FW_PATCH_DIR_ROOT=../../
BASE_FW=../../base_fw/asus_M3.12.15-A86_1032/M3.12.15-A86_1032.img
# new Firmware version string, leave empty to leave version strings untouched
FW_VERSION_STRING="SEEMOO Firmware LTE Mod"

CUSTOM_BASE_DIR=../
CUSTOM_SRCS=

SRC_DIR=./src
IMAGE_DIR=./image
BUILD_DIR=./build
PATCHED_FW_DIR=./patched
FW_BASE_DIR=./fw_base
GEN_DIR=./gen

.PHONY: clean

default: all

clean: clean_compile clean_image clean_patched_fw

include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/compile_patch.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/image/image.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/patching/bin_patch.mk
