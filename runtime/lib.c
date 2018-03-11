#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int debug = 0;

extern int64_t _LANG_NUM_T;
extern int64_t _LANG_BOOL_T;
extern int64_t _LANG_VOID_T;
extern int64_t _LANG_VEC_T;
void* _LANG_HEAP_END;

static void* buf;
static int pagesize;
static int64_t heapsize;

static void* to;
static void* from;

static void swap()
{
    void* tmp = to;
    to = from;
    from = tmp;
    mprotect(from, pagesize, PROT_READ | PROT_WRITE);
    mprotect(to, pagesize, PROT_NONE);
    _LANG_HEAP_END = from + heapsize;
}

void _lang_debug()
{
    printf("NUM_T:  %lu\n", _LANG_NUM_T);
    printf("BOOL_T: %lu\n", _LANG_BOOL_T);
    printf("VOID_T: %lu\n", _LANG_VOID_T);
    printf("VEC_T:  %lu\n", _LANG_VEC_T);
    debug = 1;
}

void _lang_print_vec(int64_t*);
static void _lang_print_num_impl(int64_t, int);
static void _lang_print_bool_impl(int64_t, int);
static void _lang_print_void_impl(int64_t, int);


void _lang_print_num(int64_t num)
{
    _lang_print_num_impl(num, 1);
}

void _lang_print_bool(int64_t num)
{
    _lang_print_bool_impl(num, 1);
}

void _lang_print_void(int64_t num)
{
    _lang_print_void_impl(num, 1);
}

static void print_all_heap(int64_t* start)
{
    debug = 0;
    while (start[0] != 0)
    {
        _lang_print_vec(start);
        int64_t* tag = (int64_t*)start[0];
        start += tag[0]+1;
    }
}

int64_t _lang_read_num()
{
    int64_t a;
    scanf("%ld", &a);
    return a;
}

// no need for new line when called by _lang_print_vec
static void _lang_print_num_impl(int64_t num, int new_line)
{
    if (debug)
    {
        print_all_heap(from);
    }
    printf("%ld", num);
    if (new_line)
    {
        printf("\n");
    }
}

static void _lang_print_bool_impl(int64_t num, int new_line)
{
    if (debug)
    {
        print_all_heap(from);
    }
    if (num == 1)
    {
        printf("True");
    }
    else if (num == 0)
    {
        printf("False");
    }
    else
    {
        printf("ERROR, expected boolean, got %ld\n", num);
    }
    if (new_line)
    {
        printf("\n");
    }
}

static void _lang_print_void_impl(int64_t num, int new_line)
{
    if (debug)
    {
        print_all_heap(from);
    }
    if (num == 0)
    {
        printf("Void");
    }
    else
    {
        printf("ERROR, expected void, got %ld\n", num);
    }
    if (new_line)
    {
        printf("\n");
    }
}
// nargs = # of entries in vector (NOT # of entries in this function)
// va_args should go:
//   type identifier
//   value
void* _lang_init_heap(int64_t heap_size)
//, int64_t root_stack_size)
{
    // init heap
    heapsize = heap_size;
    pagesize = sysconf(_SC_PAGE_SIZE);
    if (heap_size > pagesize)
    {
        printf("ERROR, heap_size too big: %lu\n", heap_size);
        exit(2);
    }
    posix_memalign(&buf, pagesize, 5 * pagesize);
    memset(buf, 0, 5 * pagesize);
    mprotect(buf, pagesize, PROT_NONE);
    mprotect(buf+pagesize, pagesize, PROT_READ | PROT_WRITE);
    mprotect(buf+2*pagesize, pagesize, PROT_NONE);
    mprotect(buf+3*pagesize, pagesize, PROT_NONE);
    mprotect(buf+4*pagesize, pagesize, PROT_NONE);
    // accessible memories:
    from = buf+2*pagesize - heapsize;
    to = buf+4*pagesize - heapsize;
    _LANG_HEAP_END = from + heapsize;
    return from;
}

void _lang_print_vec(int64_t* start)
{
    if (debug)
    {
        print_all_heap(from);
    }
    printf("Vec: ");
    int64_t* tag = (int64_t*)start[0];
    int i;
    for (i=1; i<tag[0]+1; i++)
    {
        if (tag[i] == _LANG_NUM_T)
        {
            _lang_print_num_impl(start[i], 0);
        }
        else if (tag[i] == _LANG_BOOL_T)
        {
            _lang_print_bool_impl(start[i], 0);
        }
        else if (tag[i] ==_LANG_VOID_T)
        {
            _lang_print_void_impl(start[i], 0);
        }
        else
        {
            _lang_print_vec(start+i);
        }
        printf(" ");
    }
    printf("\n");
}



void* _lang_init_rootstack(int64_t rootstack_size)
{
    return malloc(sizeof(int64_t)*rootstack_size);
}

// returns next free ptr after stop&copy
void* collect(void* root_stack_ptr, int64_t size)
{
    // for now just allocate more space
    return malloc(sizeof(int64_t)*1000);
}
