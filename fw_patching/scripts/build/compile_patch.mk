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
CFLAGS=-mv5 -Wall -Wno-attributes
LDFLAGS=-mv5 -nostdlib -nostartfiles

OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.c,%.o,$(wildcard $(SRC_DIR)/*.c))))
FW_WRAPPER=$(patsubst %.img,%_wrapper.h,$(abspath $(BASE_FW)))

.PHONY: readelf-obj disasm-obj readelf-linked disasm-linked clean_compile

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c -DFW_WRAPPER="<$(FW_WRAPPER)>" -o $@ $< $(CFLAGS)

$(PATCH_ELF): $(OBJ) $(FW_BASE_DIR)/fw_wrapper.lcs
	$(LD) -T$(FW_BASE_DIR)/fw_wrapper.lcs -T$(SEEMOO_FW_PATCH_DIR_ROOT)/scripts/build/patch.lcs $(LDFLAGS) $(OBJ) -o $(PATCH_ELF)
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
	
clean_compile:
	rm -f $(PATCH_ELF)
	rm -f $(OBJ)
	rm -f $(BUILD_DIR)/disasm*.txt
	if [ -d $(BUILD_DIR) ]; then rmdir --ignore-fail-on-non-empty $(BUILD_DIR); fi
