## @brief project Makefile
##
## This file contains project specific definitions,
## such as the used base firmware image
##
## @author Carsten Bruns (carst.bruns@gmx.de)

SEEMOO_FW_PATCH_DIR_ROOT=../../
BASE_FW=../../base_fw/xiaomi_mi4_MPSS.DI.3.0-0f11eec/MPSS.DI.3.0-0f11eec.img
# new Firmware version string, leave empty to leave version strings untouched
FW_VERSION_STRING="SEEMOO Firmware all App"

CUSTOM_BASE_DIR=../
CUSTOM_SRCS=common/seemoo_qmi/seemoo_qmi_services.c 
CUSTOM_SRCS+=func_counter_snprintf/src/func_counter_snprintf.c
CUSTOM_SRCS+=lte_mac/src/lte_mac_dl.c lte_mac/src/lte_mac_ul.c
CUSTOM_SRCS+=lte_sec/src/lte_sec.c
CUSTOM_SRCS+=mem_access/src/mem_access.c
CUSTOM_SRCS+=channel_estimation/src/channel_estimation.c

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
