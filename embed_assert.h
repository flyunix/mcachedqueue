#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <assert.h>

/*ASSERT 文本*/
#define EMBED_ASSERT(condition, fmt, args...) {\
    if(!(condition)) { \
        printf(fmt, ##args);\
        assert((condition));\
    }\
}\

/**
 * @hideinitializer
 * If ther value is non-zero, then 
 * #EMBED_ASSERT_RETURN macro will evaluate the expression in @a expr during
 * run-time. If the expression yields false, assertion will be triggered
 * and the current function will return with the specified return value.
 *
 */
#define EMBED_ASSERT_RETURN(expr,retval)    \
    do { \
        if (!(expr)) { assert(expr); return retval; } \
    } while (0)
#endif/*__ASSERT_H_*/
