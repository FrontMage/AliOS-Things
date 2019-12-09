IMG_BUILDER := $(SOURCE_ROOT)/platform/mcu/gap8/tools/gap8_flash_image_builder.py
BIN_DIR := $(OUTPUT_DIR)/binary

build_elf_flash_image:
	echo "$(BIN_DIR)/$(CLEANED_BUILD_STRING)$(RADIXPOINT)$(BINSTYPE_LOWER)$(LINK_OUTPUT_SUFFIX)"
	$(IMG_BUILDER) --verbose --flash-boot-binary=$(BIN_DIR)/$(CLEANED_BUILD_STRING)$(RADIXPOINT)$(BINSTYPE_LOWER)$(LINK_OUTPUT_SUFFIX) --raw=$(BIN_DIR)/$(CLEANED_BUILD_STRING)$(RADIXPOINT)$(BINSTYPE_LOWER).raw --flash-type=spi

EXTRA_POST_BUILD_TARGETS += build_elf_flash_image
