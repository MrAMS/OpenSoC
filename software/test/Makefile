BUILD_DIR=../../build/myplatform
SOC_DIRECTORY=../../third_party/litex/litex/soc

include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJ_DIR=./build

OBJECTS   = $(OBJ_DIR)/crt0.o
OBJECTS   += $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(wildcard *.c */*.c */*/*.c)))

all: create_build $(OBJ_DIR)/image.bin

create_build:
	@mkdir -p ./build

$(OBJ_DIR)/%.bin: $(OBJ_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

vpath %.a $(PACKAGES:%=../%)

$(OBJ_DIR)/image.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -T linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-Wl,--whole-archive \
		-Wl,--gc-sections \
		-Wl,-Map,$@.map \
		$(LIBS:lib%=-l%)

ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

$(OBJ_DIR)/donut.o: CFLAGS   += -w

VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY)

$(OBJ_DIR)/%.o: %.cpp
	$(compilexx)

$(OBJ_DIR)/%.o: %.c
	$(compile)

$(OBJ_DIR)/%.o: %.S
	$(assemble)
	@echo $(OBJECTS)

clean:
	$(RM) -r $(OBJ_DIR)

.PHONY: all clean create_build
