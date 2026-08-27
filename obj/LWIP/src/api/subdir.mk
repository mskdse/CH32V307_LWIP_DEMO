################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/src/api/api_lib.c \
../LWIP/src/api/api_msg.c \
../LWIP/src/api/err.c \
../LWIP/src/api/if_api.c \
../LWIP/src/api/netbuf.c \
../LWIP/src/api/netdb.c \
../LWIP/src/api/netifapi.c \
../LWIP/src/api/sockets.c \
../LWIP/src/api/tcpip.c 

C_DEPS += \
./LWIP/src/api/api_lib.d \
./LWIP/src/api/api_msg.d \
./LWIP/src/api/err.d \
./LWIP/src/api/if_api.d \
./LWIP/src/api/netbuf.d \
./LWIP/src/api/netdb.d \
./LWIP/src/api/netifapi.d \
./LWIP/src/api/sockets.d \
./LWIP/src/api/tcpip.d 

OBJS += \
./LWIP/src/api/api_lib.o \
./LWIP/src/api/api_msg.o \
./LWIP/src/api/err.o \
./LWIP/src/api/if_api.o \
./LWIP/src/api/netbuf.o \
./LWIP/src/api/netdb.o \
./LWIP/src/api/netifapi.o \
./LWIP/src/api/sockets.o \
./LWIP/src/api/tcpip.o 

DIR_OBJS += \
./LWIP/src/api/*.o \

DIR_DEPS += \
./LWIP/src/api/*.d \

DIR_EXPANDS += \
./LWIP/src/api/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
LWIP/src/api/%.o: ../LWIP/src/api/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307_LWIP/Debug" -I"d:/stm32project/CH32V307_LWIP/Core" -I"d:/stm32project/CH32V307_LWIP/User" -I"d:/stm32project/CH32V307_LWIP/Peripheral/inc" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/include" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307_LWIP/ETH_Driver" -I"d:/stm32project/CH32V307_LWIP/LWIP" -I"d:/stm32project/CH32V307_LWIP/LWIP/src" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include/lwip" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT/arch" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

