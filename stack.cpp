#include <stdlib.h>
#include <stdio.h>

#include "stack.h"

static int stack_ok (stack stk);
static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag);

static const char POISON     = '@';    //  '@' == 64
static const size_t MIN_SIZE = 100;
static const size_t MAX_SIZE = 20000;
static const int stack_scale = 2;

enum STACK_ERROR stack_ctor (stack* const stk, const size_t num_elem)
{
    if ((stk == NULL) || (stack_ok (*stk) != (DONE | BAD_DATA)))
    {
        return CANT_CREATE;
    }

    size_t num = (num_elem > MIN_SIZE) ? num_elem : MIN_SIZE;
    num = (num < MAX_SIZE) ? num : MAX_SIZE;

    stk->data = (stack_elem*) calloc (num, sizeof (stack_elem));
    stk->capacity = num;

    return (stk->data == NULL) ? CANT_CREATE : DONE;
}

enum STACK_ERROR stack_dtor (stack* const stk)
{
    if ((stk  == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_DESTROY;
    }

    stk->file = NULL;
    stk->line = 0;
    stk->name = NULL;
    stk->size = 0;
    stk->capacity = 0;
    free (stk->data);

    return DONE;
}

static int stack_ok (stack stk)
{
    int error = DONE;

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

    return error;
}

static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag)
{
    if (((flag != UP) && (flag != DOWN)) || (stk == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_RESIZE;
    }

    stk->capacity = (flag == UP)
                            ? stk->capacity * stack_scale
                            : stk->capacity / stack_scale;
    stk->data = (stack_elem*) realloc (stk->data, stk->capacity);

    return (stk->data == NULL) ? CANT_RESIZE : DONE;
}

enum STACK_ERROR stack_push (stack* const stk, const stack_elem element)
{
    if ((stk == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_PUSH;
    }

    enum STACK_ERROR error = DONE;

    if (stk->size == stk->capacity)
    {
        error = stack_resize (stk, UP);
    }

    stk->data [stk->size] = element;
    stk->size++;

    return error;
}

enum STACK_ERROR stack_pop (stack* const stk, stack_elem* const element)
{
    if ((stk == NULL) || (element == NULL) || (stack_ok (*stk) != DONE))
    {
        return CANT_POP;
    }

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

    return error;
}

enum STACK_ERROR dump (stack* const stk, const char* const file, const int line)
{
    if ((file == NULL) || (line <= 0))
    {
        return CANT_DUMP;
    }

    if (stk == NULL)
    {
        printf ("\nStruct has NULL pointer.\n");
        return DONE;
    }

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
