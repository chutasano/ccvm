#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

#include "rep/c0.h"
#include "rep/r0.h"
#include "rep/x0.h"
#include "rep/x0s.h"

using namespace std;

//#define RUNTIME /home/cs/lang/runtime/
#define STR(a) _STR(a)
#define _STR(a) #a

#define DEBUG

static string to_asm(r0::P &p)
{
    p.uniquify();
    p.type_check();
    c0::P c = p.flatten();
    x0s::P xs = c.select();
    x0::P x0 = xs.assign();
    x0.fix();
    string asms = x0.to_asm();
#ifdef DEBUG
    cout << asms;
#endif
    return asms;
}

string compile(r0::P &p)
{
    string woof = to_asm(p);
    string pname = "woof";
    pid_t pid;
    char namebuf[] = "/tmp/compiler-XXXXXX";
    int fd = mkstemp(namebuf);
    write(fd, (void*)woof.c_str(), woof.size());
    string name = string(namebuf) + ".s";
    // FIXME make dynamic
    string runtime = STR(RUNTIME) "lib.o";
    rename(namebuf, name.c_str());
    if ((pid = fork()) == -1)
    {
        exit(1);
    }
    else if (pid == 0)
    {
        const char* gccp = "/usr/bin/gcc";
        execl(gccp,
              gccp,
              name.c_str(),
              runtime.c_str(),
              "-o",
              pname.c_str(),
              (char*)NULL);
    }
    // TODO robust gcc error handling
    wait(NULL);
    unlink(name.c_str());
    return pname;
}

string compile_run(r0::P &p)
{
    string pname = "./" + compile(p);
    array<char, 128> buf;
    string result;
    shared_ptr<FILE> pipe(popen(pname.c_str(), "r"), pclose);
    if (!pipe)
    {
        cerr << "Failed to make pipe\n";
        exit(1);
    }
    while (!feof(pipe.get()))
    {
        if (fgets(buf.data(), 128, pipe.get()) != nullptr)
            result += buf.data();
    }
    return result; 
}

bool test_compile(const r0::P &p, int64_t expect)
{
    r0::P cpy(p);
    string output = compile_run(cpy);
    type t = static_cast<type>(cpy.t);
    cpy.deep_delete();
    size_t pos = 0;
#ifdef DEBUG
    cout << "\n\n    PROGRAM OUTPUT\n";
#endif
    while ((pos = output.find("\n")) != string::npos)
    {
        // last output line needs to be parsed for testing
        // only print and erase if it's not the last line
        if (output.find("\n", pos) == output.size()-1 ||
            output.find("\n", pos) == string::npos)
        {
            break;
        }
        else
        {
#ifdef DEBUG
            cout << output.substr(0, pos + 1);
#endif
            output.erase(0, pos + 1);
        }
    }
#ifdef DEBUG
    cout << "    PROGRAM OUTPUT END\n\n";
#endif
    if (t == TNUM)
    {
        int64_t actual = stoll(output, nullptr);
        return expect == actual;
    }
    else if (t == TBOOL)
    {
        if (output == "True\n")
        {
            return expect == TB_TRUE;
        }
        else if (output == "False\n")
        {
            return expect == TB_FALSE;
        }
        else
        {
            return false;
        }
    }
    else if (t == TVOID)
    {
        if (output == "Void\n")
        {
            return expect == TV_VOID;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

