#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_

#include <stdio.h>
#include <stdbool.h>

/*status*/
typedef int embed_status_t; 

/*large unsigned int*/
typedef size_t embed_size_t;

typedef bool embed_bool_t;

/*基本数据类型*/

#define uint8   unsigned char 
#define uint16  unsigned short  
#define uint32  unsigned int 

#if TARGET_ARM
#define sint8    signed  char
#define sint16   signed  short 
#define sint32   signed  int 
#elif TARGET_ANDROID
#define sint8    signed char
#define sint16   signed short 
#define sint32   signed int 
#endif

#define int8    char
#define int16   short 
#define int32   int 

#define success  0
#define failed   1

enum embed_constants_
{
    EMBED_SUCCESS = 0,
    EMBED_FAILD = 1
};

/** Utility macro to compute the number of elements in static array. */
#define EMBED_ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))


#define EMBED_PTR_ALIGNMENT    4

/*Test pointer p is alignment for EMBED_PTR_ALIGNMENT*/
#define IS_ALIGNED(p)   ((((unsigned long)p) & (EMBED_PTR_ALIGNMENT-1)) == 0) 

/*
** Macros to compute minimum and maximum of two numbers.
*/

#ifdef TARGET_ANDROID
#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
