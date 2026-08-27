################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/src/netif/ppp/auth.c \
../LWIP/src/netif/ppp/ccp.c \
../LWIP/src/netif/ppp/chap-md5.c \
../LWIP/src/netif/ppp/chap-new.c \
../LWIP/src/netif/ppp/chap_ms.c \
../LWIP/src/netif/ppp/demand.c \
../LWIP/src/netif/ppp/eap.c \
../LWIP/src/netif/ppp/ecp.c \
../LWIP/src/netif/ppp/eui64.c \
../LWIP/src/netif/ppp/fsm.c \
../LWIP/src/netif/ppp/ipcp.c \
../LWIP/src/netif/ppp/ipv6cp.c \
../LWIP/src/netif/ppp/lcp.c \
../LWIP/src/netif/ppp/magic.c \
../LWIP/src/netif/ppp/mppe.c \
../LWIP/src/netif/ppp/multilink.c \
../LWIP/src/netif/ppp/ppp.c \
../LWIP/src/netif/ppp/pppapi.c \
../LWIP/src/netif/ppp/pppcrypt.c \
../LWIP/src/netif/ppp/pppoe.c \
../LWIP/src/netif/ppp/pppol2tp.c \
../LWIP/src/netif/ppp/pppos.c \
../LWIP/src/netif/ppp/upap.c \
../LWIP/src/netif/ppp/utils.c \
../LWIP/src/netif/ppp/vj.c 

C_DEPS += \
./LWIP/src/netif/ppp/auth.d \
./LWIP/src/netif/ppp/ccp.d \
./LWIP/src/netif/ppp/chap-md5.d \
./LWIP/src/netif/ppp/chap-new.d \
./LWIP/src/netif/ppp/chap_ms.d \
./LWIP/src/netif/ppp/demand.d \
./LWIP/src/netif/ppp/eap.d \
./LWIP/src/netif/ppp/ecp.d \
./LWIP/src/netif/ppp/eui64.d \
./LWIP/src/netif/ppp/fsm.d \
./LWIP/src/netif/ppp/ipcp.d \
./LWIP/src/netif/ppp/ipv6cp.d \
./LWIP/src/netif/ppp/lcp.d \
./LWIP/src/netif/ppp/magic.d \
./LWIP/src/netif/ppp/mppe.d \
./LWIP/src/netif/ppp/multilink.d \
./LWIP/src/netif/ppp/ppp.d \
./LWIP/src/netif/ppp/pppapi.d \
./LWIP/src/netif/ppp/pppcrypt.d \
./LWIP/src/netif/ppp/pppoe.d \
./LWIP/src/netif/ppp/pppol2tp.d \
./LWIP/src/netif/ppp/pppos.d \
./LWIP/src/netif/ppp/upap.d \
./LWIP/src/netif/ppp/utils.d \
./LWIP/src/netif/ppp/vj.d 

OBJS += \
./LWIP/src/netif/ppp/auth.o \
./LWIP/src/netif/ppp/ccp.o \
./LWIP/src/netif/ppp/chap-md5.o \
./LWIP/src/netif/ppp/chap-new.o \
./LWIP/src/netif/ppp/chap_ms.o \
./LWIP/src/netif/ppp/demand.o \
./LWIP/src/netif/ppp/eap.o \
./LWIP/src/netif/ppp/ecp.o \
./LWIP/src/netif/ppp/eui64.o \
./LWIP/src/netif/ppp/fsm.o \
./LWIP/src/netif/ppp/ipcp.o \
./LWIP/src/netif/ppp/ipv6cp.o \
./LWIP/src/netif/ppp/lcp.o \
./LWIP/src/netif/ppp/magic.o \
./LWIP/src/netif/ppp/mppe.o \
./LWIP/src/netif/ppp/multilink.o \
./LWIP/src/netif/ppp/ppp.o \
./LWIP/src/netif/ppp/pppapi.o \
./LWIP/src/netif/ppp/pppcrypt.o \
./LWIP/src/netif/ppp/pppoe.o \
./LWIP/src/netif/ppp/pppol2tp.o \
./LWIP/src/netif/ppp/pppos.o \
./LWIP/src/netif/ppp/upap.o \
./LWIP/src/netif/ppp/utils.o \
./LWIP/src/netif/ppp/vj.o 

DIR_OBJS += \
./LWIP/src/netif/ppp/*.o \

DIR_DEPS += \
./LWIP/src/netif/ppp/*.d \

DIR_EXPANDS += \
./LWIP/src/netif/ppp/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
LWIP/src/netif/ppp/%.o: ../LWIP/src/netif/ppp/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/stm32project/CH32V307-FreeRTOS/Debug" -I"d:/stm32project/CH32V307-FreeRTOS/Core" -I"d:/stm32project/CH32V307-FreeRTOS/User" -I"d:/stm32project/CH32V307-FreeRTOS/Peripheral/inc" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/include" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"d:/stm32project/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"d:/stm32project/CH32V307-FreeRTOS/ETH_Driver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

