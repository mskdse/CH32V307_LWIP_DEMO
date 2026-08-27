################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../Startup/startup_ch32v30x_D8C.S 

S_UPPER_DEPS += \
./Startup/startup_ch32v30x_D8C.d 

OBJS += \
./Startup/startup_ch32v30x_D8C.o 

DIR_OBJS += \
./Startup/*.o \

DIR_DEPS += \
./Startup/*.d \

DIR_EXPANDS += \
./Startup/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Startup/%.o: ../Startup/%.S
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -x assembler-with-cpp -I"d:/stm32project/CH32V307-FreeRTOS/Startup" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/include" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

