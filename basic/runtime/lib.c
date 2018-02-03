#include <stdio.h>
#include <stdint.h>

int64_t _lang_read()
{
    int64_t a;
    scanf("%ld", &a);
    return a;
}

void _lang_print(int64_t num)
{
    printf("%ld\n", num);
}

