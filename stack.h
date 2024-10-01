#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

typedef char stack_elem;

#ifdef DEBUG

#define CHECKED ;                                   \
    if (my_errno != DONE)                           \
    {                                               \
        printf ("%s", decoder_error (my_errno));    \
    }

#else

#define CHECKED

#endif // NDEBUG

#define STACK_INIT(stk, number)                                                     \
    size_t stk;                                                                     \
    stack_ctor (&(stk), (number), __FILE__, __LINE__, __FUNCTION__, #stk) CHECKED;

#define STACK_DTOR(stk)         \
{                               \
    stack_dtor (stk) CHECKED;   \
    stk = 0;                    \
}

#define DUMP(stk)  dump ((stk), __FILE__, __LINE__)

#define CASE_ERROR(num_error)   \
    case num_error:             \
        return "\n" #num_error "\n"


enum STACK_ERROR
{
    DONE          = 0,
    CANT_CREATE   = 1,
    CANT_DESTROY  = 1 << 1,
    CANT_RESIZE   = 1 << 2,
    CANT_PUSH     = 1 << 3,
    CANT_POP      = 1 << 4,
    CANT_DUMP     = 1 << 5,
    BAD_STACK_PTR = 1 << 6,
    BAD_FILE_NAME = 1 << 7,
    BAD_LINE_NUM  = 1 << 8,
    BAD_NAME      = 1 << 9,
    BAD_SIZE      = 1 << 10,
    BAD_CAPACITY  = 1 << 11,
    BAD_DATA      = 1 << 12,
    BAD_STACK     = 1 << 13,
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

    #ifdef HASH_PROT

    size_t hash_data;
    size_t hash_stack;

    #endif // HASH_PROT
};

typedef struct stack_t stack;

enum STACK_ERROR stack_ctor (size_t* const stack_encode, const size_t num_elem,
                             const char* file, const int line, const char* func, const char* name);
enum STACK_ERROR stack_dtor (const size_t stack_encode);
enum STACK_ERROR stack_push (const size_t stack_encode, const stack_elem element);
enum STACK_ERROR stack_pop  (const size_t stack_encode, stack_elem* const element);
enum STACK_ERROR dump       (const size_t stack_encode, const char* const file, const int line);
const char* decoder_error   (const int error);

static enum STACK_ERROR my_errno = DONE;

#endif // STACK_H
