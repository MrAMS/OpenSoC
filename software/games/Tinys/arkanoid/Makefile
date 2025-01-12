BUILD_DIR=../../../../build/myplatform
SOC_DIRECTORY=../../../../third_party/litex/litex/soc
DRIVER_DIR=../../../driver
GAME_ENGINE_DIR=../

include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJ_DIR=./build

OBJECTS   = $(OBJ_DIR)/crt0.o
OBJECTS   += $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(wildcard *.c */*.c */*/*.c)))

OBJECTS   += $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(notdir $(abspath $(wildcard $(DRIVER_DIR)/*.c)))))

OBJECTS   += $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(notdir $(abspath $(wildcard $(GAME_ENGINE_DIR)/*.c)))))

CFLAGS	+= -I$(abspath $(DRIVER_DIR))
CFLAGS	+= -I$(abspath $(GAME_ENGINE_DIR))
CFLAGS	+= -DGE0_SCREEN_WIDTH=160 -DGE0_SCREEN_HEIGHT=80

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

VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY)

$(OBJ_DIR)/%.o: %.cpp
	$(compilexx)

$(OBJ_DIR)/%.o: %.c
	$(compile)

$(OBJ_DIR)/%.o: $(DRIVER_DIR)/%.c
	$(compile)

$(OBJ_DIR)/%.o: $(GAME_ENGINE_DIR)/%.c
	$(compile)

$(OBJ_DIR)/%.o: %.S
	$(assemble)
	@echo $(OBJECTS)

clean:
	$(RM) -r $(OBJ_DIR)

.PHONY: all clean create_build
