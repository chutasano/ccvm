#include <stdio.h>
#include <stdint.h>

int64_t _lang_read_num()
{
    int64_t a;
    scanf("%ld", &a);
    return a;
}

void _lang_print_num(int64_t num)
{
    printf("%ld\n", num);
}

void _lang_print_bool(int64_t num)
{
    if (num == 1)
    {
        printf("True\n");
    }
    else if (num == 0)
    {
        printf("False\n");
    }
    else
    {
        printf("ERROR, expected boolean, got %ld\n", num);
    }
}
