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
#include <sstream>
#include <typeinfo>

#include "rep/c0.h"
#include "rep/r0.h"
#include "rep/x0.h"
#include "rep/x0s.h"
#include "rep/type.h"
#include "test.h"

using namespace std;

#define STR(a) _STR(a)
#define _STR(a) #a

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

static bool t_num(string output, int64_t expect)
{
    int64_t actual = stoll(output, nullptr);
    return expect == actual;
}

static bool t_bool(string output, int64_t expect)
{
    // initialize actual as opposite of expect to fail by default
    tbool actual = expect == 0 ? TB_TRUE : TB_FALSE;
    if (output == "True\n" || output == "True")
    {
        actual = TB_TRUE;
    }
    else if (output == "False\n" || output == "False")
    {
        actual = TB_FALSE;
    }
    return expect == actual;
}

static bool t_void(string output, int64_t expect)
{
    if (output == "Void\n" || output == "Void")
    {
        return expect == TV_VOID;
    }
    else
    {
        return false;
    }
}

static bool t_vec(stringstream &output, vec_t expect[], int where)
{
    if (expect[where].t == TVEC)
    {
        bool status = true;
        for (int i=0; i < expect[where].val; i++)
        {
            string dummy;
            output >> dummy;
            if (dummy == "Vec:")
            {
                status &= t_vec(output, expect, where+i+1);
            }
            else
            {
                switch(expect[where+i+1].t)
                {
                    case TNUM:
                        status &= t_num(dummy, expect[where+i+1].val);
                        break;
                    case TBOOL:
                        status &= t_bool(dummy, expect[where+i+1].val);
                        break;
                    case TVOID:
                        status &= t_void(dummy, expect[where+i+1].val);
                        break;
                    default:
                        cerr << "Compile: WTF? unexpected type in expect";
                        break;
                }
            }
        }
        return status;
    }
    else
    {
        cerr << "ERROR: got a vector, expected non-vector";
        return false;
    }
}


bool test_compile(const r0::P &p, vec_t expect[])
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
    switch(t)
    {
        case TNUM:
            return t_num(output, expect[0].val);
        case TBOOL:
            return t_bool(output, expect[0].val);
        case TVOID:
            return t_void(output, expect[0].val);
        default:
            if (t > TVEC)
            {
                stringstream ss;
                ss.str(output);
                string dummy;
                ss >> dummy;
                if (dummy != "Vec:")
                {
                    cerr << "Didn't get vec from program?";
                    return false;
                }
                return t_vec(ss, expect, 0);
            }
            else
            {
                cerr << "Compile: unknown type";
                return false;
            }
    }
}

