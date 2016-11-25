WRAPPER_LCS_FILE=$(FW_BASE_DIR)/fw_wrapper.lcs
WRAPPER_HEADER=$(patsubst %.img,%_wrapper.h,$(abspath $(BASE_FW)))
 
.PHONY: clean_wrapper_lcs

$(WRAPPER_LCS_FILE): $(WRAPPER_HEADER)
	mkdir -p $(FW_BASE_DIR) 
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/create_wrapper_lcs.py $(WRAPPER_HEADER) $(WRAPPER_LCS_FILE)

clean_wrapper_lcs:
	rm -f $(WRAPPER_LCS_FILE)
