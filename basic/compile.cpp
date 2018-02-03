#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <typeinfo>

#include "rep/c0.h"
#include "rep/r0.h"
#include "rep/x0.h"
#include "rep/x0s.h"

using namespace std;

static string to_asm(r0::P p)
{
    p.uniquify();
    c0::P c = p.flatten();
    x0s::P xs = c.select();
    x0::P x0 = xs.assign();
    string asms = x0.to_asm();
    cout << asms;
    return asms;
}

void compile(r0::P p)
{
    string woof = to_asm(p);
    pid_t pid;
    char namebuf[] = "/tmp/compiler-XXXXXX";
    int fd = mkstemp(namebuf);
    write(fd, (void*)woof.c_str(), woof.size());
    string name = string(namebuf) + ".s";
    string runtime = "/home/csano/lang/basic/runtime/lib.o";
    rename(namebuf, name.c_str());
    if ((pid = fork()) == -1)
    {
        exit(1);
    }
    else if (pid == 0)
    {
        char *cname = new char[name.size()+1];
        char *cruntime = new char[runtime.size()+1];
        strncpy(cname, name.c_str(), name.size());
        strncpy(cruntime, runtime.c_str(), runtime.size());
        char* cmd[] = {
            "/usr/bin/gcc",
            cname,
            cruntime,
            "-o",
            "woof",
            NULL
        };
        execv("/usr/bin/gcc", cmd);
    }
    int meow;
    wait(&meow);
    unlink(name.c_str());
}

int compile_run(r0::P p)
{
    compile(p);
    return 0;
}

bool test_compile(r0::P p, int expect)
{
    int actual = compile_run(p);
    return expect == actual;
}
