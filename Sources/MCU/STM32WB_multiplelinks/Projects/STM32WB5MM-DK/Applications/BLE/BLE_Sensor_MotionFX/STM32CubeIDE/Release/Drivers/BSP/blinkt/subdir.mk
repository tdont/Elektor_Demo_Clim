################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/guerwood/Data/Dev/_linux/GitHub/Elektor_Demo_Clim/Sources/MCU/STM32WB_multiplelinks/Drivers/BSP/blinkt/blinkt.c 

OBJS += \
./Drivers/BSP/blinkt/blinkt.o 

C_DEPS += \
./Drivers/BSP/blinkt/blinkt.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/blinkt/blinkt.o: /media/guerwood/Data/Dev/_linux/GitHub/Elektor_Demo_Clim/Sources/MCU/STM32WB_multiplelinks/Drivers/BSP/blinkt/blinkt.c Drivers/BSP/blinkt/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g -DUSE_STM32WB5M_DK -DSTM32WB5Mxx -DUSE_HAL_DRIVER -c -I../../Core/Inc -I../../STM32_WPAN/App -I../../../../../../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../../../../../../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../../../../../../../Utilities/lpm/tiny_lpm -I../../../../../../../Middlewares/ST/STM32_WPAN -I../../../../../../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../../../../../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../../../../../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../../../../../../Middlewares/ST/STM32_WPAN/utilities -I../../../../../../../Middlewares/ST/STM32_WPAN/ble/core -I../../../../../../../Middlewares/ST/STM32_WPAN/ble/core/auto -I../../../../../../../Middlewares/ST/STM32_WPAN/ble/core/template -I../../../../../../../Middlewares/ST/STM32_WPAN/ble/svc/Inc -I../../../../../../../Middlewares/ST/STM32_WPAN/ble/svc/Src -I../../../../../../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../../../../../../Utilities/sequencer -I../../../../../../../Middlewares/ST/STM32_WPAN/ble -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/BSP/STM32WB5MM-DK -I../../../../../../../Drivers/BSP/Components/ssd1315 -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Utilities/LCD -I../../../../../../../Drivers/BSP/Components/ism330dhcx -I../../../../../../../Drivers/CMSIS/DSP/Source/FilteringFunctions -I../../../../../../../Drivers/CMSIS/DSP/Include -I../../../../../../../Drivers/BSP/Components/stts22h -I../../../../../../../Drivers/BSP/Components/s25fl128s -I../../../../../../../Middlewares/ST/STM32_MotionFX_Library/Inc -I../../Core/Src/vl53l0x -I../../../../../../../Drivers/BSP/blinkt -I../../../../../../../3rdparty/YACSGL/inc -I../../../../../../../3rdparty/YACSWL/inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-blinkt

clean-Drivers-2f-BSP-2f-blinkt:
	-$(RM) ./Drivers/BSP/blinkt/blinkt.cyclo ./Drivers/BSP/blinkt/blinkt.d ./Drivers/BSP/blinkt/blinkt.o ./Drivers/BSP/blinkt/blinkt.su

.PHONY: clean-Drivers-2f-BSP-2f-blinkt

