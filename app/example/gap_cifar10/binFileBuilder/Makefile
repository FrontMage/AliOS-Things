BUILD_TARGET=FlashBuild
OUTPUT=*.dat

C_FLAGS += -m32
C_FLAGS += -Wno-discarded-qualifiers -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
C_FLAGS += -D__EMUL__ -DFlashFile

LIBS += -lm -L/usr/lib32

all:
	gcc $(C_FLAGS) FlashBuild.c -I$(TARGET_INSTALL_DIR)/include -I../dumpCoef -o $(BUILD_TARGET) $(LIBS)

run:
	./$(BUILD_TARGET)

clean:
	rm -rf $(BUILD_TARGET) $(OUTPUT)
