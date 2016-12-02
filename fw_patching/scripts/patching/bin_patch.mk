## @file bin_patch.mk
## @brief Makefile to handle combination of the patch code with the base firmware ELF file
##
## @author Carsten Bruns (carst.bruns@gmx.de)

PATCH_ELF=$(BUILD_DIR)/fw_patch.elf
FW_BASE_ELF=$(FW_BASE_DIR)/fw_base.elf
PATCHED_ELF=$(PATCHED_FW_DIR)/patched.elf
FW_WRAPPER=$(patsubst %.img,%_wrapper.h,$(abspath $(BASE_FW)))
export PYTHONPATH=$(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/utils

.PHONY: clean_patched_fw

$(PATCHED_FW_DIR)/patched.elf: $(PATCH_ELF) $(FW_BASE_ELF)
	mkdir -p $(PATCHED_FW_DIR)
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/patching/patcher.py $(FW_BASE_ELF) $(PATCH_ELF) $(PATCHED_ELF) $(FW_WRAPPER) $(SRC_DIR) $(FW_VERSION_STRING)
	
clean_patched_fw:
	rm -f $(PATCHED_FW_DIR)/*.elf
	if [ -d $(PATCHED_FW_DIR) ]; then rmdir --ignore-fail-on-non-empty $(PATCHED_FW_DIR); fi
