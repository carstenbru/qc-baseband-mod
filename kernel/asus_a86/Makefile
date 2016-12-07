## @brief Android boot image Makefile
##
## This file contains rules to compile the Android kernel
## and embed it into a bootable image file.
##
## @author Carsten Bruns (carst.bruns@gmx.de)

KERNEL=PadFoneT004-10.12.3.94-kernel-src
DEFCONFIG=a86_seemoo_defconfig

MKBOOT=$(shell pwd)/buildtools/mkboot/mkboot
BASE_FILES=./base_files
BOOTIMG_TMP=./bootimg_tmp
KERNEL_PATCH_DIR=./patches

INCLUDE_PATH_FIX=-Idrivers/media/platform/msm/camera_v2/sensor -Idrivers/power/service -Isound/soc/msm/qdsp6v2

all: boot.img

.PHONY: patch_kernel clean clean_boot clean_kernel check-setup-env

clean: clean_boot clean_kernel

patch_kernel: $(KERNEL)/Makefile $(wildcard $(KERNEL_PATCH_DIR)/**/*)
	cp -a $(KERNEL_PATCH_DIR)/* $(KERNEL)/

$(KERNEL)/Makefile:
	#extract kernel
	mkdir -p ./$(KERNEL)
	tar -xf $(BASE_FILES)/$(KERNEL).tar.gz --directory=./$(KERNEL)
	#patch timeconst.pl file for python 2.7
	sed -i 's/defined(@val)/@val/g' ./$(KERNEL)/kernel/timeconst.pl
	#patch Makefile to include local directory (for includes with <>)
	sed -i '/^KBUILD_AFLAGS_KERNEL :=/a\KBUILD_CFLAGS += $(INCLUDE_PATH_FIX)' ./$(KERNEL)/Makefile
	
$(KERNEL)/.config: patch_kernel check-setup-env
	cd $(KERNEL) && make $(DEFCONFIG)

$(KERNEL)/arch/arm/boot/zImage: FORCE $(KERNEL)/Makefile patch_kernel $(KERNEL)/.config check-setup-env
	cd $(KERNEL) && make

boot.img: $(KERNEL)/arch/arm/boot/zImage check-setup-env
	rm -Rf $(BOOTIMG_TMP)
	#unpack source boot image and replace kernel
	$(MKBOOT) $(BASE_FILES)/boot.img $(BOOTIMG_TMP)
	rm -f $(BOOTIMG_TMP)/kernel
	cp $(KERNEL)/arch/arm/boot/zImage $(BOOTIMG_TMP)/kernel
	
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
