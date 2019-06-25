IMG_BUILDER := $(SOURCE_ROOT)/platform/mcu/gap8/tools/gap8_flash_image_builder.py
BIN_DIR := $(OUTPUT_DIR)/binary
MODELS_DIR := $(SOURCE_ROOT)/app/example/gap_face_reid/models

FLASH_BINS := --comp=$(MODELS_DIR)/conv1.bias.bin\
   --comp=$(MODELS_DIR)/conv1.weights.bin\
   --comp=$(MODELS_DIR)/features.0.bias.bin\
   --comp=$(MODELS_DIR)/features.0.weights.bin\
   --comp=$(MODELS_DIR)/features.10.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.10.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.10.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.10.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.10.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.10.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.11.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.11.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.11.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.11.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.11.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.11.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.12.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.12.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.12.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.12.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.12.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.12.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.3.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.3.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.3.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.3.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.3.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.3.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.4.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.4.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.4.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.4.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.4.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.4.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.6.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.6.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.6.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.6.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.6.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.6.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.7.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.7.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.7.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.7.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.7.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.7.squeeze.weights.bin\
   --comp=$(MODELS_DIR)/features.9.expand1x1.bias.bin\
   --comp=$(MODELS_DIR)/features.9.expand1x1.weights.bin\
   --comp=$(MODELS_DIR)/features.9.expand3x3.bias.bin\
   --comp=$(MODELS_DIR)/features.9.expand3x3.weights.bin\
   --comp=$(MODELS_DIR)/features.9.squeeze.bias.bin\
   --comp=$(MODELS_DIR)/features.9.squeeze.weights.bin

build_full_flash_image: build_elf_flash_image
	$(IMG_BUILDER) --verbose --flash-boot-binary=$(BIN_DIR)/$(CLEANED_BUILD_STRING)$(RADIXPOINT)$(BINSTYPE_LOWER)$(LINK_OUTPUT_SUFFIX) --raw=$(BIN_DIR)/$(CLEANED_BUILD_STRING)$(RADIXPOINT)$(BINSTYPE_LOWER).raw $(FLASH_BINS)

EXTRA_POST_BUILD_TARGETS += build_full_flash_image
