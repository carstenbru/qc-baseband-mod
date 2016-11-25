
export MTOOLS_SKIP_CHECK=1
FW_BASE_ELF=$(FW_BASE_DIR)/fw_base.elf
PATCHED_MODEM_IMG=$(IMAGE_DIR)/patched_modem.img
BLOB_DIR=$(PATCHED_FW_DIR)/blobs/
PATCHED_ELF=$(PATCHED_FW_DIR)/patched.elf

.PHONY: all fix_hash_table clean_image

all: $(PATCHED_MODEM_IMG)

$(FW_BASE_ELF):
	mkdir -p $(FW_BASE_DIR)
	rm -f $(FW_BASE_DIR)/modem.*
	mcopy -i $(BASE_FW) ::image/modem.* $(FW_BASE_DIR)
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/image/blob-merge.py $(FW_BASE_DIR)/modem $(FW_BASE_ELF)
	rm -f $(FW_BASE_DIR)/modem.*
	
$(BLOB_DIR)/modem.mdt: $(PATCHED_ELF)
	mkdir -p $(BLOB_DIR)
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/image/pil-splitter.py $(PATCHED_ELF) $(BLOB_DIR)/modem
	
fix_hash_table: $(BLOB_DIR)/modem.mdt
	python $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/image/pil-patcher.py -u -m $(BLOB_DIR)/modem
	
$(PATCHED_MODEM_IMG): fix_hash_table
	mkdir -p $(IMAGE_DIR)
	cp -f $(BASE_FW) $(PATCHED_MODEM_IMG)
	mcopy -D o -i $(PATCHED_MODEM_IMG) $(BLOB_DIR)/modem.* ::image/
	
clean_image:
	rm -f $(FW_BASE_DIR)/modem.*
	rm -f $(FW_BASE_ELF)
	rm -f $(BLOB_DIR)/modem.*
	rm -f $(PATCHED_MODEM_IMG)
	if [ -d $(BLOB_DIR) ]; then rmdir --ignore-fail-on-non-empty $(BLOB_DIR); fi
	if [ -d $(FW_BASE_DIR) ]; then rmdir --ignore-fail-on-non-empty $(FW_BASE_DIR); fi
	if [ -d $(IMAGE_DIR) ]; then rmdir --ignore-fail-on-non-empty $(IMAGE_DIR); fi
	
