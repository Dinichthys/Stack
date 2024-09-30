#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

typedef char stack_elem;

#define INIT(stk)  {__FILE__, __LINE__, __FUNCTION__, #stk, 0, 0, NULL}

#define DUMP(stk)  dump ((stk), __FILE__, __LINE__)

#define CASE_ERROR(num_error)   \
    case num_error:             \
        return #num_error

enum STACK_ERROR
{
    DONE          = 0,
    CANT_CREATE   = 1,
    CANT_DESTROY  = 1 << 1,
    CANT_RESIZE   = 1 << 2,
    CANT_PUSH     = 1 << 3,
    CANT_POP      = 1 << 4,
    CANT_DUMP     = 1 << 5,
    BAD_FILE_NAME = 1 << 6,
    BAD_LINE_NUM  = 1 << 7,
    BAD_NAME      = 1 << 8,
    BAD_SIZE      = 1 << 9,
    BAD_CAPACITY  = 1 << 10,
    BAD_DATA      = 1 << 11,
    BAD_STACK     = 1 << 12,
};

enum RESIZE_DIRECTION
{
    DOWN = 0,
    UP   = 1,
};

struct stack_t
{
    const char* file;

    int line;

    const char* func;

    const char* name;

    size_t size;

    size_t capacity;

    stack_elem* data;
};

typedef struct stack_t stack;

enum STACK_ERROR stack_ctor   (stack* const stk, const size_t num_elem);
enum STACK_ERROR stack_dtor   (stack* const stk);
enum STACK_ERROR stack_push   (stack* const stk, const stack_elem element);
enum STACK_ERROR stack_pop    (stack* const stk, stack_elem* const element);
enum STACK_ERROR dump         (stack* const stk, const char* const file, const int line);
const char* decoder_error     (const int error);

#endif // STACK_H
