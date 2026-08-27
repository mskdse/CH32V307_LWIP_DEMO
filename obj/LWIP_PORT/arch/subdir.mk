################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP_PORT/arch/sys_arch.c 

C_DEPS += \
./LWIP_PORT/arch/sys_arch.d 

OBJS += \
./LWIP_PORT/arch/sys_arch.o 

DIR_OBJS += \
./LWIP_PORT/arch/*.o \

DIR_DEPS += \
./LWIP_PORT/arch/*.d \

DIR_EXPANDS += \
./LWIP_PORT/arch/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
LWIP_PORT/arch/%.o: ../LWIP_PORT/arch/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307-FreeRTOS/Debug" -I"d:/stm32project/CH32V307-FreeRTOS/Core" -I"d:/stm32project/CH32V307-FreeRTOS/User" -I"d:/stm32project/CH32V307-FreeRTOS/Peripheral/inc" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/include" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307-FreeRTOS/ETH_Driver" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP/src" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP/src/include" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP/src/include/lwip" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP_PORT" -I"d:/stm32project/CH32V307-FreeRTOS/LWIP_PORT/arch" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

