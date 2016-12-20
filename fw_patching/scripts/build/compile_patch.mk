## @file compile_patch.mk
## @brief Makefile to handle patch compilation and linking against a base firmware wrapper (fw_wrapper.lcs)
##
## For debugging purposes this file also includes targets to read ELF information 
## and display the disassembled patch code.
##
## @author Carsten Bruns (carst.bruns@gmx.de)

PATCH_ELF=$(BUILD_DIR)/fw_patch.elf

CC=hexagon-gcc
LD=hexagon-ld
RELF=hexagon-readelf
OBJDUMP=hexagon-objdump
EDITOR=kwrite
CFLAGS=-mv5 -O2 -G0 -Wall -Wno-attributes
LDFLAGS=-mv5 -nostdlib -nostartfiles

CUSTOM_OBJS=$(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(CUSTOM_SRCS)))

SRC_FILES_PROJECT=$(abspath $(wildcard $(SRC_DIR)/*.c))
SRC_FILES_CUSTOM=$(addprefix $(CUSTOM_BASE_DIR)/,$(CUSTOM_SRCS))
SRC_FILES=$(SRC_FILES_PROJECT) $(SRC_FILES_CUSTOM)
OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.c,%.o,$(SRC_FILES_PROJECT))))
OBJ+=$(CUSTOM_OBJS)
FW_IMG_WRAPPER=$(abspath $(patsubst %.img,%_wrapper.h,$(abspath $(BASE_FW))))

WRAPPER_LCS_FILE=$(FW_BASE_DIR)/fw_wrapper.lcs

FW_ORG=fw_org_functions
FW_ORG_OBJ=$(BUILD_DIR)/$(FW_ORG).o
FW_ORG_LCS=$(GEN_DIR)/$(FW_ORG).lcs
FW_ORG_HEADER=$(abspath $(GEN_DIR)/$(FW_ORG).h)

include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/create_wrapper_lcs.mk
include $(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/generate_fw_org_functions.mk

.PHONY: readelf-obj disasm-obj readelf-linked disasm-linked clean_compile

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(FW_ORG_OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) -c -DFW_WRAPPER="<$(FW_ORG_HEADER)>" -DFW_IMG_WRAPPER="<$(FW_IMG_WRAPPER)>" -o $@ $< $(CFLAGS)
	
$(BUILD_DIR)/%.o: $(CUSTOM_BASE_DIR)/%.c $(FW_ORG_OBJ)
	mkdir -p $(dir $@)
	$(CC) -c -DFW_WRAPPER="<$(FW_ORG_HEADER)>" -DFW_IMG_WRAPPER="<$(FW_IMG_WRAPPER)>" -o $@ $< $(CFLAGS)
	
$(BUILD_DIR)/%.o: $(GEN_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PATCH_ELF): $(OBJ) $(FW_ORG_OBJ) $(WRAPPER_LCS_FILE) $(FW_ORG_SRC)
	$(LD) -T$(WRAPPER_LCS_FILE) -T$(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/patch.lcs -T$(FW_ORG_LCS) $(LDFLAGS) $(OBJ) $(FW_ORG_OBJ) -o $(PATCH_ELF)
	@echo patch ELF file generated

readelf-obj: $(OBJ)
	$(RELF) -a $(OBJ)

disasm-obj:$(OBJ)
	$(OBJDUMP) -D $(OBJ) > $(BUILD_DIR)/disasm_obj.txt
	$(EDITOR) $(BUILD_DIR)/disasm_obj.txt &

readelf-linked: $(PATCH_ELF)
	$(RELF) -a $(PATCH_ELF)

disasm-linked: $(PATCH_ELF)
	$(OBJDUMP) -D $(PATCH_ELF) > $(BUILD_DIR)/disasm_linked.txt
	$(EDITOR) $(BUILD_DIR)/disasm_linked.txt &
	
clean_compile: clean_wrapper_lcs clean_fw_org_functions
	rm -f $(PATCH_ELF)
	rm -f $(OBJ)
	rm -f $(FW_ORG_OBJ)
	rm -f $(BUILD_DIR)/disasm*.txt
	rm -rf $(BUILD_DIR)
