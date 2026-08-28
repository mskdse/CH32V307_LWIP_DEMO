################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ETH_Driver/eth_driver_10M.c \
../ETH_Driver/eth_driver_RMII.c 

C_DEPS += \
./ETH_Driver/eth_driver_10M.d \
./ETH_Driver/eth_driver_RMII.d 

OBJS += \
./ETH_Driver/eth_driver_10M.o \
./ETH_Driver/eth_driver_RMII.o 

DIR_OBJS += \
./ETH_Driver/*.o \

DIR_DEPS += \
./ETH_Driver/*.d \

DIR_EXPANDS += \
./ETH_Driver/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
ETH_Driver/%.o: ../ETH_Driver/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307_LWIP/Debug" -I"d:/stm32project/CH32V307_LWIP/Core" -I"d:/stm32project/CH32V307_LWIP/User" -I"d:/stm32project/CH32V307_LWIP/Peripheral/inc" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/include" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307_LWIP/ETH_Driver" -I"d:/stm32project/CH32V307_LWIP/LWIP" -I"d:/stm32project/CH32V307_LWIP/LWIP/src" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include/lwip" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT/arch" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

