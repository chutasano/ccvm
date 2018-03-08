#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

extern int64_t _LANG_NUM_T;
extern int64_t _LANG_BOOL_T;
extern int64_t _LANG_VOID_T;
extern int64_t _LANG_VEC_T;

static void* buf;
static int pagesize;

static void* to;
static void* from;

static void swap()
{
    void* tmp = to;
    to = from;
    from = tmp;
    mprotect(from, pagesize, PROT_READ | PROT_WRITE);
    mprotect(to, pagesize, PROT_NONE);
}

void _lang_debug()
{
    printf("NUM_T:  %lu\n", _LANG_NUM_T);
    printf("BOOL_T: %lu\n", _LANG_BOOL_T);
    printf("VOID_T: %lu\n", _LANG_VOID_T);
    printf("VEC_T:  %lu\n", _LANG_VEC_T);
}

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

void _lang_print_void(int64_t num)
{
    if (num == 0)
    {
        printf("Void\n");
    }
    else
    {
        printf("ERROR, expected void, got %ld\n", num);
    }
}
// nargs = # of entries in vector (NOT # of entries in this function)
// va_args should go:
//   type identifier
//   value
void* _lang_init_heap(int64_t heap_size, int64_t root_stack_size)
{
    // init heap
    pagesize = sysconf(_SC_PAGE_SIZE);
    if (heap_size > pagesize)
    {
        printf("ERROR, heap_size too big: %lu\n", heap_size);
        exit(2);
    }
    posix_memalign(&buf, pagesize, 5 * pagesize);
    mprotect(buf, pagesize, PROT_NONE);
    mprotect(buf+2*pagesize, pagesize, PROT_NONE);
    mprotect(buf+3*pagesize, pagesize, PROT_NONE);
    mprotect(buf+4*pagesize, pagesize, PROT_NONE);
    // accessible memories:
    from = buf+2*pagesize - heap_size;
    to = buf+4*pagesize - heap_size;
    mprotect(from, pagesize, PROT_READ | PROT_WRITE);
    return malloc(sizeof(void*)*root_stack_size);
}

//void collect(void* root_stack_ptr, )
