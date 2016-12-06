#/**************************************************************************
#*                                                                         *
#*          ###########   ###########   ##########    ##########           *
#*         ############  ############  ############  ############          *
#*         ##            ##            ##   ##   ##  ##        ##          *
#*         ##            ##            ##   ##   ##  ##        ##          *
#*         ###########   ####  ######  ##   ##   ##  ##    ######          *
#*          ###########  ####  #       ##   ##   ##  ##    #    #          *
#*                   ##  ##    ######  ##   ##   ##  ##    #    #          *
#*                   ##  ##    #       ##   ##   ##  ##    #    #          *
#*         ############  ##### ######  ##   ##   ##  ##### ######          *
#*         ###########    ###########  ##   ##   ##   ##########           *
#*                                                                         *
#*            S E C U R E   M O B I L E   N E T W O R K I N G              *
#*                                                                         *
#* Warning:                                                                *
#*                                                                         *
#* Our software may damage your hardware and may void your hardware’s      *
#* warranty! You use our tools at your own risk and responsibility!        *
#*                                                                         *
#* License:                                                                *
#* Copyright (c) 2015 NexMon Team                                          *
#*                                                                         *
#* Permission is hereby granted, free of charge, to any person obtaining   *
#* a copy of this software and associated documentation files (the         *
#* "Software"), to deal in the Software without restriction, including     *
#* without limitation the rights to use, copy, modify, merge, publish,     *
#* distribute copies of the Software, and to permit persons to whom the    *
#* Software is furnished to do so, subject to the following conditions:    *
#*                                                                         *
#* The above copyright notice and this permission notice shall be included *
#* in all copies or substantial portions of the Software.                  *
#*                                                                         *
#* Any use of the Software which results in an academic publication or     *
#* other publication which includes a bibliography must include a citation *
#* to the author's publication "M. Schulz, D. Wegemer and M. Hollick.      *
#* NexMon: A Cookbook for Firmware Modifications on Smartphones to Enable  *
#* Monitor Mode.".                                                         *
#*                                                                         *
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS *
#* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF              *
#* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  *
#* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    *
#* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    *
#* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       *
#* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  *
#*                                                                         *
#**************************************************************************/

KERNEL=PadFoneT004-10.12.3.94-kernel-src

MKBOOT=$(shell pwd)/buildtools/mkboot/mkboot

all: boot.img

.PHONY: cleanall cleanboot kernel

clean: cleanboot

kernel: $(KERNEL)/arch/arm/boot/zImage

$(KERNEL)/arch/arm/boot/zImage: FORCE $(KERNEL)/.config check-setup-env
	cd $(KERNEL) && make

boot.img: $(KERNEL)/arch/arm/boot/zImage check-setup-env
	rm -Rf bootimg_tmp
	#unpack source boot image and replace kernel
	$(MKBOOT) bootimg_src/boot.img bootimg_tmp
	rm -f bootimg_tmp/kernel
	cp $(KERNEL)/arch/arm/boot/zImage bootimg_tmp/kernel
	
	#build new boot image
	$(MKBOOT) bootimg_tmp boot.img

cleanboot:
	rm -Rf bootimg_tmp
	rm -f boot.img

boot: boot.img
	adb reboot bootloader
	fastboot boot boot.img

check-setup-env:
ifndef QC_BASEBAND_SETUP_ENV
	$(error run 'source setup_env.sh' first)
endif
	
FORCE:
