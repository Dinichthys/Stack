#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "stack.h"

#ifdef HASH_PROT

#include "hash.h"

#endif // HASH_PROT

#ifdef CANARY_PROT

static bool check_canary (stack* const stk);

#endif // CANARY_PROT

static int stack_ok (stack* const stk);
static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag);

#ifdef CANARY_PROT

static const size_t CANARY = 1910;

#endif // CANARY_PROT

static const char POISON     = '@';    //  '@' == 64
static const size_t MIN_SIZE = 100;
static const size_t MAX_SIZE = 20000;
static const int STACK_SCALE = 2;
static const size_t KEY      = (size_t) time (NULL);

#ifdef CANARY_PROT

static bool check_canary (stack* const stk)
{
    return (memcmp ((char*) stk - sizeof (CANARY), &CANARY, sizeof (CANARY)) != 0)
        || (memcmp ((char*) stk + sizeof (stack), &CANARY, sizeof (CANARY)) != 0)
        || (memcmp ((char*) (stk->data) - sizeof (CANARY), &CANARY, sizeof (CANARY)) != 0)
        || (memcmp ((char*) (stk->data) + stk->capacity * sizeof (stack_elem), &CANARY, sizeof (CANARY)) != 0);
}

#endif // CANARY_PROT

enum STACK_ERROR stack_ctor (size_t* const stack_encode, const size_t num_elem,
                             const char* file, const int line, const char* func, const char* name)
{
    if (stack_encode == NULL)
    {
        return CANT_CREATE;
    }

    #ifdef CANARY_PROT

    stack* const stk = (stack*) ((char*) calloc (1, sizeof (stack) + 2 * sizeof (CANARY)) + sizeof (CANARY));

    if ((char*) stk - sizeof (CANARY) == NULL)
    {
        return CANT_CREATE;
    }

    memcpy ((char*) stk - sizeof (CANARY), &CANARY, sizeof (CANARY));
    memcpy ((char*) stk + sizeof (stack), &CANARY, sizeof (CANARY));

    #else // CANARY_PROT

    stack* const stk = (stack*) calloc (1, sizeof (stack));

    if (stk == NULL)
    {
        return CANT_CREATE;
    }

    #endif // CANARY_PROT

    *stk = {file, line, func, name, 0, 0, NULL};

    size_t num = (num_elem > MIN_SIZE) ? num_elem : MIN_SIZE;
    num = (num < MAX_SIZE) ? num : MAX_SIZE;

    #ifdef CANARY_PROT

    stk->data = (stack_elem*) ((char*) calloc (num, sizeof (stack_elem) + 2 * sizeof (CANARY)) + sizeof (CANARY));

    if ((char*) stk->data - sizeof (CANARY) == NULL)
    {
        return CANT_CREATE;
    }

    memcpy ((char*) (stk->data) - sizeof (CANARY), &CANARY, sizeof (CANARY));
    memcpy ((char*) (stk->data) + num * sizeof (stack_elem), &CANARY, sizeof (CANARY));

    #else // CANARY_PROT

    stk->data = (stack_elem*) calloc (num, sizeof (stack_elem));

    if (stk->data == NULL)
    {
        return CANT_CREATE;
    }

    #endif // CANARY_PROT

    stk->capacity = num;

    for (size_t i = 0; i < stk->capacity; i++)
    {
        stk->data [i] = POISON;
    }

    *stack_encode = ((size_t) stk) ^ KEY;

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_CREATE;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    stk->hash_data = hashing (stk->data, stk->capacity * sizeof (stack_elem));

    stk->hash_stack = hashing (stk, sizeof (stack) - sizeof (stk->hash_stack));

    #endif // HASH_PROT

    return DONE;
}

enum STACK_ERROR stack_dtor (const size_t stack_encode)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk  == NULL) || (stack_ok (stk) != DONE))
    {
        return CANT_DESTROY;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_DESTROY;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return CANT_DESTROY;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return CANT_DESTROY;
    }

    #endif // HASH_PROT

    stk->file = NULL;
    stk->line = 0;
    stk->name = NULL;
    stk->size = 0;
    stk->capacity = 0;

    #ifdef CANARY_PROT

    free ((char*) (stk->data) - sizeof (CANARY));

    #else // CANARY_PROT

    free (stk->data);

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    stk->hash_data = 0;
    stk->hash_stack = 0;

    #endif // HASH_PROT

    #ifdef CANARY_PROT

    free ((char*) (stk) - sizeof (CANARY));

    #else // CANARY_PROT

    free (stk);

    #endif // CANARY_PROT

    return DONE;
}

static int stack_ok (stack* const stk)
{
    int error = DONE;

    if (stk == NULL)
    {
        return BAD_STACK;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return BAD_STACK;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return BAD_STACK;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return BAD_STACK;
    }

    #endif // HASH_PROT

    if (stk->file == NULL)
    {
        error |=  BAD_FILE_NAME;
    }

    if (stk->line == 0)
    {
        error |= BAD_LINE_NUM;
    }

    if (stk->name == NULL)
    {
        error |= BAD_NAME;
    }

    if ((stk->size > MAX_SIZE) || (stk->size > stk->capacity))
    {
        error |= BAD_SIZE;
    }

    if ((stk->capacity > MAX_SIZE) || (stk->size > stk->capacity))
    {
        error |= BAD_CAPACITY;
    }

    if (stk->data == NULL)
    {
        error |= BAD_DATA;
    }

    if ((error & BAD_FILE_NAME) && (error & BAD_LINE_NUM) && (error & BAD_NAME)
          && (error & BAD_SIZE) && (error & BAD_CAPACITY) && (error & BAD_DATA))
    {
        error |= BAD_STACK;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return BAD_STACK;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return BAD_STACK;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return BAD_STACK;
    }

    #endif // HASH_PROT

    return error;
}

static enum STACK_ERROR stack_resize (stack* const stk, const enum RESIZE_DIRECTION flag)
{
    if (((flag != UP) && (flag != DOWN)) || (stk == NULL) || (stack_ok (stk) != DONE))
    {
        return CANT_RESIZE;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_RESIZE;
    }

    const size_t location_canary = stk->capacity;

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return CANT_RESIZE;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return CANT_RESIZE;
    }

    #endif // HASH_PROT

    stk->capacity = (flag == UP)
                            ? stk->capacity * STACK_SCALE
                            : stk->capacity / STACK_SCALE;

    #ifdef CANARY_PROT

    stk->data = (stack_elem*) ((char*) realloc ((char*) stk->data - sizeof (CANARY),
                                                stk->capacity * sizeof (stack_elem) + 2 * sizeof (CANARY))
                                                + sizeof (CANARY));

    if ((char*) stk->data - sizeof (CANARY) == NULL)
    {
        CANT_RESIZE;
    }

    if (flag == UP)
    {
        size_t zero = 0UL;
        memcpy ((char*) (stk->data) + location_canary * sizeof (stack_elem), &zero, sizeof (zero));
    }
    memcpy ((char*) (stk->data) + stk->capacity * sizeof (stack_elem), &CANARY, sizeof (CANARY));

    #else // CANARY_PROT

    stk->data = (stack_elem*) realloc (stk->data, stk->capacity * sizeof (stack_elem));

    if (stk->data == NULL)
    {
        return CANT_RESIZE;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    stk->hash_data = hashing (stk->data, stk->capacity * sizeof (stack_elem));

    stk->hash_stack = hashing (stk, sizeof (stack) - sizeof (stk->hash_stack));

    #endif // HASH_PROT

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_RESIZE;
    }

    #endif // CANARY_PROT

    return DONE;
}

enum STACK_ERROR stack_push (const size_t stack_encode, const stack_elem element)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk == NULL) || (stack_ok (stk) != DONE))
    {
        return CANT_PUSH;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_PUSH;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return CANT_PUSH;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return CANT_PUSH;
    }

    #endif // HASH_PROT

    enum STACK_ERROR error = DONE;

    if (stk->size == stk->capacity)
    {
        error = stack_resize (stk, UP);
    }

    stk->data [stk->size] = element;
    stk->size++;

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_PUSH;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    stk->hash_data = hashing (stk->data, stk->capacity * sizeof (stack_elem));

    #endif // HASH_PROT

    return error;
}

enum STACK_ERROR stack_pop (const size_t stack_encode, stack_elem* const element)
{
    stack* const stk = (stack*) (stack_encode ^ KEY);

    if ((stk == NULL) || (element == NULL) || (stack_ok (stk) != DONE))
    {
        return CANT_POP;
    }

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_POP;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return CANT_POP;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return CANT_POP;
    }

    #endif // HASH_PROT

    enum STACK_ERROR error = DONE;

    if (stk->size == 0)
    {
        return CANT_POP;
    }

    if (stk->size == stk->capacity / 4)
    {
        error = stack_resize (stk, DOWN);
    }

    *element = stk->data [stk->size - 1];
    stk->data [stk->size - 1] = POISON;
    stk->size--;

    #ifdef HASH_PROT

    stk->hash_data = hashing (stk->data, stk->capacity * sizeof (stack_elem));

    stk->hash_stack = hashing (stk, sizeof (stack) - sizeof (stk->hash_stack));

    #endif // HASH_PROT

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_POP;
    }

    #endif // CANARY_PROT

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

    #ifdef CANARY_PROT

    if (check_canary (stk))
    {
        return CANT_DUMP;
    }

    #endif // CANARY_PROT

    #ifdef HASH_PROT

    if (stk->hash_stack != hashing (stk, sizeof (stack) - sizeof (stk->hash_stack)))
    {
        return CANT_DUMP;
    }

    if (stk->hash_data != hashing (stk->data, stk->capacity * sizeof (stack_elem)))
    {
        return CANT_DUMP;
    }

    #endif // HASH_PROT

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
