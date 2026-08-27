################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/src/core/altcp.c \
../LWIP/src/core/altcp_alloc.c \
../LWIP/src/core/altcp_tcp.c \
../LWIP/src/core/def.c \
../LWIP/src/core/dns.c \
../LWIP/src/core/inet_chksum.c \
../LWIP/src/core/init.c \
../LWIP/src/core/ip.c \
../LWIP/src/core/mem.c \
../LWIP/src/core/memp.c \
../LWIP/src/core/netif.c \
../LWIP/src/core/pbuf.c \
../LWIP/src/core/raw.c \
../LWIP/src/core/stats.c \
../LWIP/src/core/sys.c \
../LWIP/src/core/tcp.c \
../LWIP/src/core/tcp_in.c \
../LWIP/src/core/tcp_out.c \
../LWIP/src/core/timeouts.c \
../LWIP/src/core/udp.c 

C_DEPS += \
./LWIP/src/core/altcp.d \
./LWIP/src/core/altcp_alloc.d \
./LWIP/src/core/altcp_tcp.d \
./LWIP/src/core/def.d \
./LWIP/src/core/dns.d \
./LWIP/src/core/inet_chksum.d \
./LWIP/src/core/init.d \
./LWIP/src/core/ip.d \
./LWIP/src/core/mem.d \
./LWIP/src/core/memp.d \
./LWIP/src/core/netif.d \
./LWIP/src/core/pbuf.d \
./LWIP/src/core/raw.d \
./LWIP/src/core/stats.d \
./LWIP/src/core/sys.d \
./LWIP/src/core/tcp.d \
./LWIP/src/core/tcp_in.d \
./LWIP/src/core/tcp_out.d \
./LWIP/src/core/timeouts.d \
./LWIP/src/core/udp.d 

OBJS += \
./LWIP/src/core/altcp.o \
./LWIP/src/core/altcp_alloc.o \
./LWIP/src/core/altcp_tcp.o \
./LWIP/src/core/def.o \
./LWIP/src/core/dns.o \
./LWIP/src/core/inet_chksum.o \
./LWIP/src/core/init.o \
./LWIP/src/core/ip.o \
./LWIP/src/core/mem.o \
./LWIP/src/core/memp.o \
./LWIP/src/core/netif.o \
./LWIP/src/core/pbuf.o \
./LWIP/src/core/raw.o \
./LWIP/src/core/stats.o \
./LWIP/src/core/sys.o \
./LWIP/src/core/tcp.o \
./LWIP/src/core/tcp_in.o \
./LWIP/src/core/tcp_out.o \
./LWIP/src/core/timeouts.o \
./LWIP/src/core/udp.o 

DIR_OBJS += \
./LWIP/src/core/*.o \

DIR_DEPS += \
./LWIP/src/core/*.d \

DIR_EXPANDS += \
./LWIP/src/core/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
LWIP/src/core/%.o: ../LWIP/src/core/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307_LWIP/Debug" -I"d:/stm32project/CH32V307_LWIP/Core" -I"d:/stm32project/CH32V307_LWIP/User" -I"d:/stm32project/CH32V307_LWIP/Peripheral/inc" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/include" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307_LWIP/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307_LWIP/ETH_Driver" -I"d:/stm32project/CH32V307_LWIP/LWIP" -I"d:/stm32project/CH32V307_LWIP/LWIP/src" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include" -I"d:/stm32project/CH32V307_LWIP/LWIP/src/include/lwip" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT" -I"d:/stm32project/CH32V307_LWIP/LWIP_PORT/arch" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

