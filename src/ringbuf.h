#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>
#include <string.h>

#define RINGBUF_PRIVATE_MAKE_STRUCT_NAME(type, size) \
    ringbuf_ ## type ## _ ## size ## _s

#define RINGBUF_DEFINE(type, size) \
    struct RINGBUF_PRIVATE_MAKE_STRUCT_NAME(type,size) \
    { \
         size_t rpos; \
         size_t wpos; \
         size_t num; \
         type buf[size > 255 * sizeof(size_t) ? -1 : size]; \
    }

#define RINGBUF_PRIVATE_MAKE_NAME(name) ringbuf_ ## name
    
#define RINGBUF_CREATE(type, size, name, opt) \
    opt struct RINGBUF_PRIVATE_MAKE_STRUCT_NAME(type,size) \
        RINGBUF_PRIVATE_MAKE_NAME(name) = {0,0,size}

#define RINGBUF_INIT(name) \
    memset(&RINGBUF_PRIVATE_MAKE_NAME(name), 0, sizeof(RINGBUF_PRIVATE_MAKE_NAME(name)))
    
#define RINGBUF_WRITE(name, val) \
    RINGBUF_PRIVATE_MAKE_NAME(name).buf[RINGBUF_PRIVATE_MAKE_NAME(name).wpos] = val; \
    RINGBUF_PRIVATE_MAKE_NAME(name).wpos = (RINGBUF_PRIVATE_MAKE_NAME(name).wpos + 1) \
        % RINGBUF_PRIVATE_MAKE_NAME(name).num

#define RINGBUF_READ(name, var) \
    var = RINGBUF_PRIVATE_MAKE_NAME(name).buf[RINGBUF_PRIVATE_MAKE_NAME(name).rpos]; \
        
#define RINGBUF_MARK(name, mark) \
    RINGBUF_PRIVATE_MAKE_NAME(name).buf[RINGBUF_PRIVATE_MAKE_NAME(name).rpos] = mark; \
    RINGBUF_PRIVATE_MAKE_NAME(name).rpos = (RINGBUF_PRIVATE_MAKE_NAME(name).rpos + 1) \
        % RINGBUF_PRIVATE_MAKE_NAME(name).num
    
#endif
