#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "stack.h"

static int stack_ok (stack stk);
static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag);

#ifdef CANARY

static const size_t CANARY = 1910;

#endif // CANARY

static const char POISON     = '@';    //  '@' == 64
static const size_t MIN_SIZE = 100;
static const size_t MAX_SIZE = 20000;
static const int STACK_SCALE = 2;
static const size_t KEY      = (size_t) time (NULL);

enum STACK_ERROR stack_ctor (size_t* const stack_encode, const size_t num_elem,
                             const char* file, const int line, const char* func, const char* name)
{
    if (stack_encode == NULL)
    {
        return CANT_CREATE;
    }

    #ifdef CANARY

    stack* const stk = (stack*) ((char*) calloc (1, sizeof (stack) + 2 * sizeof (CANARY)) + sizeof (CANARY));
    *((size_t*) ((char*) stk - sizeof (CANARY))) = CANARY;
    *((size_t*) ((char*) stk + sizeof (stack))) = CANARY;

    #else

    stack* const stk = (stack*) calloc (1, sizeof (stack));

    #endif // CANARY


    if (stk == NULL)
    {
        return CANT_CREATE;
    }

    *stk = {file, line, func, name, 0, 0, NULL};

    if (stack_ok (*stk) != (DONE | BAD_DATA))
    {
        return CANT_CREATE;
    }

    size_t num = (num_elem > MIN_SIZE) ? num_elem : MIN_SIZE;
    num = (num < MAX_SIZE) ? num : MAX_SIZE;

    #ifdef CANARY

    stk->data = (stack_elem*) ((char*) calloc (num, sizeof (stack_elem) + 2 * sizeof (CANARY)) + sizeof (CANARY));
    *((size_t*) ((char*) (stk->data) - sizeof (CANARY))) = CANARY;
    *((size_t*) ((char*) (stk->data) + num * sizeof (stack_elem))) = CANARY;


    #else

    stk->data = (stack_elem*) calloc (num, sizeof (stack_elem));

    #endif // CANARY
    stk->capacity = num;

    *stack_encode = ((size_t) stk) ^ KEY;

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_CREATE;
    }

    #endif // CANARY

    #ifdef HASH

    char* hash_ptr = (char*) stk;

    while (hash_ptr < stk + sizeof (stack) - sizeof (stack->hash_stack))
    {
        stk->hash_stack += *hash_ptr;
    }

    #else


    return (stk->data == NULL) ? CANT_CREATE : DONE;
}

enum STACK_ERROR stack_dtor (const size_t stack_encode)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk  == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_DESTROY;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_DESTROY;
    }

    #endif // CANARY

    stk->file = NULL;
    stk->line = 0;
    stk->name = NULL;
    stk->size = 0;
    stk->capacity = 0;
    free (stk->data);
    free (stk);

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_DESTROY;
    }

    #endif // CANARY

    return DONE;
}

static int stack_ok (stack stk)
{
    int error = DONE;

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return BAD_STACK;
    }

    #endif // CANARY

    if (stk.file == NULL)
    {
        error |=  BAD_FILE_NAME;
    }

    if (stk.line == 0)
    {
        error |= BAD_LINE_NUM;
    }

    if (stk.name == NULL)
    {
        error |= BAD_NAME;
    }

    if ((stk.size > MAX_SIZE) || (stk.size > stk.capacity))
    {
        error |= BAD_SIZE;
    }

    if ((stk.capacity > MAX_SIZE) || (stk.size > stk.capacity))
    {
        error |= BAD_CAPACITY;
    }

    if (stk.data == NULL)
    {
        error |= BAD_DATA;
    }

    if ((error & BAD_FILE_NAME) && (error & BAD_LINE_NUM) && (error & BAD_NAME)
          && (error & BAD_SIZE) && (error & BAD_CAPACITY) && (error & BAD_DATA))
    {
        error |= BAD_STACK;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return BAD_STACK;
    }

    #endif // CANARY

    return error;
}

static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag)
{
    if (((flag != UP) && (flag != DOWN)) || (stk == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_RESIZE;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_RESIZE;
    }

    const size_t location_canary = stk->capacity;

    #endif // CANARY


    stk->capacity = (flag == UP)
                            ? stk->capacity * STACK_SCALE
                            : stk->capacity / STACK_SCALE;

    #ifdef CANARY

    stk->data = (stack_elem*) ((char*) realloc (stk->data, stk->capacity * sizeof (stack_elem) + 2 * sizeof (CANARY)) + sizeof (CANARY));
    if (flag == up)
    {
        *((size_t*) ((char*) (stk->data) + location_canary * sizeof (stack_elem))) = 0;
    }
    *((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY;

    #else

    stk->data = (stack_elem*) realloc (stk->data, stk->capacity * sizeof (stack_elem));

    #endif // CANARY

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_RESIZE;
    }

    #endif // CANARY

    return (stk->data == NULL) ? CANT_RESIZE : DONE;
}

enum STACK_ERROR stack_push (const size_t stack_encode, const stack_elem element)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_PUSH;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_PUSH;
    }

    #endif // CANARY

    enum STACK_ERROR error = DONE;

    if (stk->size == stk->capacity)
    {
        error = stack_resize (stk, UP);
    }

    stk->data [stk->size] = element;
    stk->size++;

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_PUSH;
    }

    #endif // CANARY

    return error;
}

enum STACK_ERROR stack_pop (const size_t stack_encode, stack_elem* const element)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk == NULL) || (element == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_POP;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_POP;
    }

    #endif // CANARY

    enum STACK_ERROR error = DONE;

    if (stk->size == stk->capacity / 4)
    {
        error = stack_resize (stk, DOWN);
    }

    if (stk->size == 1)
    {
        return CANT_POP;
    }

    *element = stk->data [stk->size - 1];
    stk->data [stk->size - 1] = POISON;
    stk->size--;

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_POP;
    }

    #endif // CANARY

    return error;
}

enum STACK_ERROR dump (const size_t stack_encode, const char* const file, const int line)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((file == NULL) || (line <= 0))
    {
        return CANT_DUMP;
    }

    if (stk == NULL)
    {
        printf ("\nStruct has NULL pointer.\n");
        return DONE;
    }

    #ifdef CANARY

    if ((*((size_t*) ((char*) stk - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) stk + sizeof (stack))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) - sizeof (CANARY))) != CANARY)
        || (*((size_t*) ((char*) (stk->data) + stk->capacity * sizeof (stack_elem))) = CANARY))
    {
        return CANT_DUMP;
    }

    #endif // CANARY

    if (stk->name == NULL)
    {
        printf ("\nStruct has invalid pointer for name but has own pointer");
    }
    else
    {
        printf ("\nStruct %s", stk->name);
    }

    printf (" [%p] at %s:%d", stk, file, line);

    if (stk->file == NULL)
    {
        printf (" hasn't name of file where it was born\n");
    }
    else
    {
        printf (" was born at %s", stk->file);
    }

    if (stk->line <= 0)
    {
        printf (" but the number of line where it was born was lost.");
    }
    else
    {
        printf (":%d", stk->line);
    }

    if (stk->func == NULL)
    {
        printf (" but the function where it was born is undefined\n");
    }
    else
    {
        printf (" (%s)\n", stk->func);
    }

    if (stk->capacity == 0)
    {
        printf ("Struct hasn\'t any elements.\n");
        return DONE;
    }

    if ((stk->size > stk->capacity) || (stk->size > MAX_SIZE))
    {
        printf ("Struct has invalid information about size and capacity.\n");
        return DONE;
    }

    printf ("{\n");

    for (size_t i = 0; i < stk->size; i++)
    {
        printf ("\t#[%lu] ", i);
        size_t j = 0;
        while (j < sizeof (stack_elem))
        {
            char* symbol = ((char*) (stk->data) + i * sizeof (stack_elem) + j);
            printf ("%c ", *symbol);
            j++;
        }
        printf ("\n");
    }

    for (size_t i = stk->size; (i < MAX_SIZE) && (i < stk->capacity); i++)
    {
        printf ("\t [%lu] @ POISON\n", i);
    }

    printf ("}\n");

    if (MAX_SIZE < stk->capacity)
    {
        printf (" %lu Stack has much more elements than MAX_SIZE\n", stk->capacity);
    }

    return DONE;
}

const char* decoder_error (const int error)
{
    switch (error)
    {
        CASE_ERROR (DONE);
        CASE_ERROR (CANT_CREATE);
        CASE_ERROR (CANT_DESTROY);
        CASE_ERROR (CANT_RESIZE);
        CASE_ERROR (CANT_PUSH);
        CASE_ERROR (CANT_POP);
        CASE_ERROR (CANT_DUMP);
        default:
            return "Invalid value for error";
    }
}
