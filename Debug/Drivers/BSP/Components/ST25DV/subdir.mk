################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/ST25DV/st25dv.c \
../Drivers/BSP/Components/ST25DV/st25dv_reg.c 

OBJS += \
./Drivers/BSP/Components/ST25DV/st25dv.o \
./Drivers/BSP/Components/ST25DV/st25dv_reg.o 

C_DEPS += \
./Drivers/BSP/Components/ST25DV/st25dv.d \
./Drivers/BSP/Components/ST25DV/st25dv_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/ST25DV/%.o Drivers/BSP/Components/ST25DV/%.su Drivers/BSP/Components/ST25DV/%.cyclo: ../Drivers/BSP/Components/ST25DV/%.c Drivers/BSP/Components/ST25DV/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L073xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"/home/kevin/STM32CubeIDE/workspace_eren/sht43/NFC/Target" -I"/home/kevin/STM32CubeIDE/workspace_eren/sht43/Middlewares/ST/STM32_NFC/Inc" -I../Drivers/BSP/Components/ST25DV -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-ST25DV

clean-Drivers-2f-BSP-2f-Components-2f-ST25DV:
	-$(RM) ./Drivers/BSP/Components/ST25DV/st25dv.cyclo ./Drivers/BSP/Components/ST25DV/st25dv.d ./Drivers/BSP/Components/ST25DV/st25dv.o ./Drivers/BSP/Components/ST25DV/st25dv.su ./Drivers/BSP/Components/ST25DV/st25dv_reg.cyclo ./Drivers/BSP/Components/ST25DV/st25dv_reg.d ./Drivers/BSP/Components/ST25DV/st25dv_reg.o ./Drivers/BSP/Components/ST25DV/st25dv_reg.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-ST25DV

