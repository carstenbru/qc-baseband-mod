## @file generate_fw_org_functions.mk
## @brief Makefile to generate functions to call an original (and overwritten) function in the firmware
##
## Three files are generated:
##	fw_org_functions.c:	function definitions
##	fw_org_functions.h:	function declartations header
##	fw_org_functions.lcs:	linker script defining symbols used in function definitions
##
## @author Carsten Bruns (carst.bruns@gmx.de)

FW_ORG_SRC=$(GEN_DIR)/$(FW_ORG).c

FW_BASE_ELF=$(FW_BASE_DIR)/fw_base.elf
 
.PHONY: clean_fw_org_functions

$(FW_ORG_SRC): $(WRAPPER_LCS_FILE) $(FW_BASE_ELF) $(SRC_FILES)
	echo $(SRC_FILES)
	mkdir -p $(GEN_DIR) 
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/generate_fw_org_functions.py $(FW_ORG_SRC) $(FW_ORG_HEADER) $(FW_ORG_LCS) $(SYMTAB_JSON_FILE) $(FW_BASE_ELF) $(SRC_FILES)

clean_fw_org_functions:
	rm -f $(FW_ORG_SRC)
	rm -f $(FW_ORG_HEADER)
	rm -f $(FW_ORG_LCS)
	if [ -d $(GEN_DIR) ]; then rmdir --ignore-fail-on-non-empty $(GEN_DIR); fi
	
