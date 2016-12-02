## @file create_wrapper_lcs.mk
## @brief Makefile to generate the base firmware wrapper (fw_wrapper.lcs)
##
## The generated linker script contains only symbol definitions for the base firmware.
## These are obtained from the firmware wrapper header file.
##
## @author Carsten Bruns (carst.bruns@gmx.de)

WRAPPER_LCS_FILE=$(FW_BASE_DIR)/fw_wrapper.lcs
SYMTAB_JSON_FILE=$(FW_BASE_DIR)/symtab.json
WRAPPER_HEADER=$(patsubst %.img,%_wrapper.h,$(abspath $(BASE_FW)))
 
.PHONY: clean_wrapper_lcs

$(WRAPPER_LCS_FILE): $(WRAPPER_HEADER)
	mkdir -p $(FW_BASE_DIR) 
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/create_wrapper_lcs.py $(WRAPPER_HEADER) $(WRAPPER_LCS_FILE) $(SYMTAB_JSON_FILE)

clean_wrapper_lcs:
	rm -f $(WRAPPER_LCS_FILE)
	rm -f $(SYMTAB_JSON_FILE)
