## @brief Android boot image Makefile
##
## This file contains rules to compile the Android kernel
## and embed it into a bootable image file.
##
## @author Carsten Bruns (carst.bruns@gmx.de)

KERNEL=Xiaomi_Kernel_OpenSource
DEFCONFIG=cancro_seemoo_defconfig

MKBOOT=$(shell pwd)/../buildtools/mkboot/mkboot
BASE_FILES=./base_files
BOOTIMG_TMP=./bootimg_tmp
KERNEL_PATCH_DIR=./patches
RAMDISK_PATCH_DIR=./patches_ramdisk

INCLUDE_PATH_FIX=-Idrivers/media/platform/msm/camera_v2/sensor -Idrivers/power/service -Isound/soc/msm/qdsp6v2 -Iarch/arm/mach-msm -Idrivers/tspdrv

KERNEL_PATCH_FILES=$(shell find $(KERNEL_PATCH_DIR) -name '*.patch')
RAMDISK_PATCH_FILES=$(shell find $(RAMDISK_PATCH_DIR) -name '*.patch')

all: boot.img

.PHONY: patch_kernel clean clean_boot clean_kernel check-setup-env

clean: clean_boot clean_kernel 

# applies a set of patch files to a target directory
# e.g. $(call apply_patches,$(RAMDISK_PATCH_FILES),$(BOOTIMG_TMP)/ramdisk)
define apply_patches
	for patchfile in $(1) ; do \
		echo "Patch file: $$patchfile" ;\
		if (cd $(2) && patch --dry-run --reverse -N -p0 -r -) < $$patchfile >/dev/null 2>&1; then \
			echo "Patch already applied - skipping." ;\
		else\
			echo "Patching..." ;\
			(cd $(2) && patch -N -p0 -r -) < $$patchfile ;\
		fi ;\
	done
endef

patch_kernel: $(KERNEL)/Makefile $(wildcard $(KERNEL_PATCH_DIR)/*/*)
	@if [ -d $(KERNEL_PATCH_DIR) ]; then rsync -a --exclude='*.patch' $(KERNEL_PATCH_DIR)/ $(KERNEL)/; fi
	@if [ -d $(KERNEL_PATCH_DIR) ]; then $(call apply_patches,$(KERNEL_PATCH_FILES),$(KERNEL)); fi
	
$(KERNEL)/Makefile:
	#get kernel source code
	git clone https://github.com/MiCode/Xiaomi_Kernel_OpenSource.git --branch cancro-kk-oss
	#patch timeconst.pl file for python 2.7
	sed -i 's/defined(@val)/@val/g' ./$(KERNEL)/kernel/timeconst.pl
	#patch Makefile to include local directory (for includes with <>)
	sed -i '/^KBUILD_AFLAGS_KERNEL :=/a\KBUILD_CFLAGS += $(INCLUDE_PATH_FIX)' ./$(KERNEL)/Makefile
	#remove -Wall flag from KBUILD_CFLAGS to avoid forbidden warnings on compilation (problem: drivers/cpufreq/cpufreq_interactive.c)
	sed -i 's/-Wall//g' ./$(KERNEL)/Makefile
	
$(KERNEL)/.config: patch_kernel check-setup-env
	cd $(KERNEL) && make $(DEFCONFIG)

$(KERNEL)/arch/arm/boot/zImage: FORCE $(KERNEL)/Makefile patch_kernel $(KERNEL)/.config check-setup-env
	cd $(KERNEL) && make CONFIG_NO_ERROR_ON_MISMATCH=y -j4
	
boot.img: $(KERNEL)/arch/arm/boot/zImage check-setup-env
	rm -Rf $(BOOTIMG_TMP)
	#unpack source boot image and replace kernel
	$(MKBOOT) $(BASE_FILES)/boot.img $(BOOTIMG_TMP)
	@if [ -d $(RAMDISK_PATCH_DIR) ]; then rsync -a --exclude='*.patch' $(RAMDISK_PATCH_DIR) $(BOOTIMG_TMP)/ramdisk; fi
	@if [ -d $(RAMDISK_PATCH_DIR) ]; then $(call apply_patches,$(RAMDISK_PATCH_FILES),$(BOOTIMG_TMP)/ramdisk); fi
	
	rm -f $(BOOTIMG_TMP)/kernel
	cp $(KERNEL)/arch/arm/boot/zImage-dtb $(BOOTIMG_TMP)/kernel
	
	#build new boot image
	$(MKBOOT) $(BOOTIMG_TMP) boot.img

clean_boot:
	rm -Rf $(BOOTIMG_TMP)
	rm -f boot.img
	
clean_kernel:
	rm -rf ./$(KERNEL)

check-setup-env:
ifndef QC_BASEBAND_SETUP_ENV
	$(error run 'source setup_env.sh' first)
endif
	
FORCE:
