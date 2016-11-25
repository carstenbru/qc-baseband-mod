SEEMOO_FW_PATCH_DIR_ROOT=../../
BASE_FW=../../base_fw/a86_orig.img
# new Firmware version string, leave empty to leave version strings untouched
FW_VERSION_STRING="SEEMOO Firmware Modding"

SRC_DIR=./src
IMAGE_DIR=./image
BUILD_DIR=./build
PATCHED_FW_DIR=./patched
FW_BASE_DIR=./fw_base

.PHONY: clean

default: all

clean: clean_wrapper_lcs clean_compile clean_image clean_patched_fw

include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/create_wrapper_lcs.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/compile_patch.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/image/image.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/patching/bin_patch.mk
