################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Utilities/subdir.mk
-include Middlewares/STM32_WPAN/subdir.mk
-include Drivers/STM32WBxx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/blinkt/subdir.mk
-include Drivers/BSP/STM32WB5MM-DK/subdir.mk
-include Drivers/BSP/Components/subdir.mk
-include Application/User/STM32_WPAN/Target/subdir.mk
-include Application/User/STM32_WPAN/App/subdir.mk
-include Application/User/Core/vl53l0x/subdir.mk
-include Application/User/Core/subdir.mk
-include Application/Startup/subdir.mk
-include 3rdparty/YACSWL/src/subdir.mk
-include 3rdparty/YACSGL/src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := BLE_Sensor_MotionFX
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
BLE_Sensor_MotionFX.elf \

MAP_FILES += \
BLE_Sensor_MotionFX.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
BLE_Sensor_MotionFX.list \

OBJCOPY_HEX += \
BLE_Sensor_MotionFX.hex \


# All Target
all: main-build

# Main-build Target
main-build: BLE_Sensor_MotionFX.elf secondary-outputs

# Tool invocations
BLE_Sensor_MotionFX.elf BLE_Sensor_MotionFX.map: $(OBJS) $(USER_OBJS) /media/guerwood/Data/Dev/_linux/GitHub/Elektor_Demo_Clim/Sources/MCU/STM32WB_multiplelinks/Projects/STM32WB5MM-DK/Applications/BLE/BLE_Sensor_MotionFX/STM32CubeIDE/STM32WB5MMGHX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "BLE_Sensor_MotionFX.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/media/guerwood/Data/Dev/_linux/GitHub/Elektor_Demo_Clim/Sources/MCU/STM32WB_multiplelinks/Projects/STM32WB5MM-DK/Applications/BLE/BLE_Sensor_MotionFX/STM32CubeIDE/STM32WB5MMGHX_FLASH.ld" --specs=nosys.specs -Wl,-Map="BLE_Sensor_MotionFX.map" -Wl,--gc-sections -Wl,--verbose -static -L../../../../../../../Middlewares/ST/STM32_MotionFX_Library/Lib --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

BLE_Sensor_MotionFX.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "BLE_Sensor_MotionFX.list"
	@echo 'Finished building: $@'
	@echo ' '

BLE_Sensor_MotionFX.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "BLE_Sensor_MotionFX.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) BLE_Sensor_MotionFX.elf BLE_Sensor_MotionFX.hex BLE_Sensor_MotionFX.list BLE_Sensor_MotionFX.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
