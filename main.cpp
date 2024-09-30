#include <stdio.h>

#include "stack.h"

int main ()
{
    stack stk = INIT (stk);

    enum STACK_ERROR error = DONE;

    error = stack_ctor (&stk, 10);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 'r');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 't');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, ' ');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 's');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 't');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 'a');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 'c');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_push (&stk, 'k');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = DUMP (&stk);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    char execute_1 = '\0';
    char execute_2 = '\0';

    error = stack_pop (&stk, &execute_1);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_pop (&stk, &execute_2);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    printf ("\n execute_1 = %c \n execute_2 = %c \n", execute_1, execute_2);

    error = stack_push (&stk, 'r');
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = DUMP (&stk);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }

    error = stack_dtor (&stk);
    if (error != DONE)
    {
        printf (decoder_error (error));
    }
}
