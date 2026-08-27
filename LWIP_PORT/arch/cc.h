/**
 * cc.h - LwIP 编译器适配层 for CH32V307
 */

#ifndef __CC_H__
#define __CC_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* ====================================================
   1. 数据类型定义
   ==================================================== */
typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;
typedef uintptr_t mem_ptr_t;

/* ====================================================
   2. 字节序 (只在未定义时定义，避免重定义警告)
   ==================================================== */
#ifndef BYTE_ORDER
#define BYTE_ORDER               LITTLE_ENDIAN
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN            1
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN               0
#endif

/* ====================================================
   3. 编译器属性
   ==================================================== */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT       __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x)     x

#define LWIP_NORETURN            __attribute__((noreturn))

/* ====================================================
   4. 断言和错误处理
   ==================================================== */
#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x)  do { \
    printf("Assertion failed: %s\r\n", x); \
    while(1); \
} while(0)
#endif

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x)    do { printf x; } while(0)
#endif

/* ====================================================
   5. 内存操作
   ==================================================== */
#define LWIP_RAND()              ((u32_t)rand())

/* ====================================================
   6. 防止timeval重定义
   ==================================================== */
#ifndef LWIP_TIMEVAL_PRIVATE
#define LWIP_TIMEVAL_PRIVATE     0
#endif

/* ====================================================
   7. 内联函数
   ==================================================== */
#define LWIP_INLINE              static inline

/* ====================================================
   8. 调试宏 (生产环境关闭)
   ==================================================== */
#define LWIP_DEBUG              0

#endif /* __CC_H__ */