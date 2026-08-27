################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/croutine.c \
../FreeRTOS/event_groups.c \
../FreeRTOS/list.c \
../FreeRTOS/queue.c \
../FreeRTOS/stream_buffer.c \
../FreeRTOS/tasks.c \
../FreeRTOS/timers.c 

C_DEPS += \
./FreeRTOS/croutine.d \
./FreeRTOS/event_groups.d \
./FreeRTOS/list.d \
./FreeRTOS/queue.d \
./FreeRTOS/stream_buffer.d \
./FreeRTOS/tasks.d \
./FreeRTOS/timers.d 

OBJS += \
./FreeRTOS/croutine.o \
./FreeRTOS/event_groups.o \
./FreeRTOS/list.o \
./FreeRTOS/queue.o \
./FreeRTOS/stream_buffer.o \
./FreeRTOS/tasks.o \
./FreeRTOS/timers.o 

DIR_OBJS += \
./FreeRTOS/*.o \

DIR_DEPS += \
./FreeRTOS/*.d \

DIR_EXPANDS += \
./FreeRTOS/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/%.o: ../FreeRTOS/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307_LWIP/Debug" -I"d:/stm32project/CH32V307_LWIP/Core" -I"d:/stm32project/CH32V307_LWIP/User" -I"d:/stm32project/CH32V307_LWIP/Peripheral/inc" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/include" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307_LWIP/ETH_Driver" -I"d:/stm32project/CH32V307_LWIP/LWIP" -I"d:/stm32project/CH32V307_LWIP/LWIP/src" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include/lwip" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT/arch" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

