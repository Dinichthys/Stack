#include <stdio.h>

#include "stack.h"

int main ()
{

    STACK_INIT (stk, 10);

    my_errno = stack_push (stk, 'r') CHECKED;

    my_errno = stack_push (stk, 't') CHECKED;

    my_errno = stack_push (stk, ' ') CHECKED;

    my_errno = stack_push (stk, 's') CHECKED;

    my_errno = stack_push (stk, 't') CHECKED;

    my_errno = stack_push (stk, 'a') CHECKED;

    my_errno = stack_push (stk, 'c') CHECKED;

    my_errno = stack_push (stk, 'k') CHECKED;

    my_errno = DUMP (stk) CHECKED;

    char execute_1 = '\0';
    char execute_2 = '\0';

    my_errno = stack_pop (stk, &execute_1) CHECKED;

    my_errno = stack_pop (stk, &execute_2) CHECKED;

    printf ("\n execute_1 = %c \n execute_2 = %c \n", execute_1, execute_2);

    my_errno = stack_push (stk, 'r') CHECKED;

    my_errno = DUMP (stk) CHECKED;

    STACK_DTOR (stk);

    STACK_INIT (test_stk, 1);

    for (int i = 0; i < 1000; i++)
    {
        my_errno = stack_push (test_stk, 'a');
    }

    my_errno = DUMP (test_stk) CHECKED;

    char c = '\0';

    for (int i = 0; i < 1001; i++)
    {
        my_errno = stack_pop (test_stk, &c) CHECKED;
    }

    my_errno = DUMP (test_stk) CHECKED;

    STACK_DTOR (test_stk);
}
