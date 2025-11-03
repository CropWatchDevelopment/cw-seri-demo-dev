################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../NFC/Target/lib_NDEF_config.c 

OBJS += \
./NFC/Target/lib_NDEF_config.o 

C_DEPS += \
./NFC/Target/lib_NDEF_config.d 


# Each subdirectory must supply rules for building sources it contributes
NFC/Target/%.o NFC/Target/%.su NFC/Target/%.cyclo: ../NFC/Target/%.c NFC/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L073xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"/home/kevin/STM32CubeIDE/workspace_eren/sht43/NFC/Target" -I"/home/kevin/STM32CubeIDE/workspace_eren/sht43/Middlewares/ST/STM32_NFC/Inc" -I../Drivers/BSP/Components/ST25DV -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-NFC-2f-Target

clean-NFC-2f-Target:
	-$(RM) ./NFC/Target/lib_NDEF_config.cyclo ./NFC/Target/lib_NDEF_config.d ./NFC/Target/lib_NDEF_config.o ./NFC/Target/lib_NDEF_config.su

.PHONY: clean-NFC-2f-Target

