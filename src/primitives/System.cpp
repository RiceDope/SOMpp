/*
 *
 *
 Copyright (c) 2007 Michael Haupt, Tobias Pape, Arne Bergmann
 Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
 http://www.hpi.uni-potsdam.de/swa/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "System.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

#include "../memory/Heap.h"
#include "../misc/defs.h"
#include "../primitivesCore/PrimitiveContainer.h"
#include "../primitivesCore/Routine.h"
#include "../vm/Globals.h"
#include "../vm/Print.h"
#include "../vm/Universe.h"
#include "../vmobjects/ObjectFormats.h"
#include "../vmobjects/VMClass.h"
#include "../vmobjects/VMFrame.h"
#include "../vmobjects/VMString.h"
#include "../vmobjects/VMSymbol.h"

#if defined(__GNUC__)

  #include <sys/time.h>

#else

  #include <misc/gettimeofday.h>

#endif

_System* System_;

static vm_oop_t sysGlobal_(vm_oop_t, vm_oop_t rightObj) {
    VMSymbol* arg = static_cast<VMSymbol*>(rightObj);
    vm_oop_t result = Universe::GetGlobal(arg);

    return result ? result : load_ptr(nilObject);
}

static vm_oop_t sysGlobalPut(vm_oop_t sys, vm_oop_t sym, vm_oop_t val) {
    VMSymbol* arg = static_cast<VMSymbol*>(sym);
    Universe::SetGlobal(arg, val);
    return sys;
}

static vm_oop_t sysHasGlobal_(vm_oop_t, vm_oop_t rightObj) {
    VMSymbol* arg = static_cast<VMSymbol*>(rightObj);

    if (Universe::HasGlobal(arg)) {
        return load_ptr(trueObject);
    } else {
        return load_ptr(falseObject);
    }
}

static vm_oop_t sysLoad_(vm_oop_t, vm_oop_t rightObj) {
    VMSymbol* arg = static_cast<VMSymbol*>(rightObj);
    VMClass* result = Universe::LoadClass(arg);
    if (result) {
        return result;
    } else {
        return load_ptr(nilObject);
    }
}

static vm_oop_t sysExit_(vm_oop_t, vm_oop_t err) {
    long err_no = INT_VAL(err);
    Quit(err_no);
}

static vm_oop_t sysPrintString_(vm_oop_t leftObj, vm_oop_t rightObj) {
    VMString* arg = static_cast<VMString*>(rightObj);
    std::string str = arg->GetStdString();
    Print(str);
    return leftObj;
}

static vm_oop_t sysPrintNewline(vm_oop_t leftObj) {
    Print("\n");
    return leftObj;
}

static vm_oop_t sysErrorPrint_(vm_oop_t leftObj, vm_oop_t rightObj) {
    VMString* arg = static_cast<VMString*>(rightObj);
    std::string str = arg->GetStdString();
    ErrorPrint(str);
    return leftObj;
}

static vm_oop_t sysErrorPrintNewline_(vm_oop_t leftObj, vm_oop_t rightObj) {
    VMString* arg = static_cast<VMString*>(rightObj);
    std::string str = arg->GetStdString();
    ErrorPrint(str + "\n");
    return leftObj;
}

static struct timeval start_time;

static vm_oop_t sysTime(vm_oop_t) {
    struct timeval now;

    gettimeofday(&now, nullptr);

    long long diff = ((now.tv_sec - start_time.tv_sec) * 1000) +   // seconds
                     ((now.tv_usec - start_time.tv_usec) / 1000);  // useconds

    return NEW_INT(diff);
}

static vm_oop_t sysTicks(vm_oop_t) {
    struct timeval now;

    gettimeofday(&now, nullptr);

    long long diff =
        ((now.tv_sec - start_time.tv_sec) * 1000 * 1000) +  // seconds
        ((now.tv_usec - start_time.tv_usec));               // useconds

    return NEW_INT(diff);
}

static vm_oop_t sysFullGC(vm_oop_t) {
    // not safe to do it immediatly, will be done when it is ok, i.e., in the
    // interpreter loop
    GetHeap<HEAP_CLS>()->requestGC();
    return load_ptr(trueObject);
}

static vm_oop_t sysLoadFile_(vm_oop_t, vm_oop_t rightObj) {
    VMString* fileName = static_cast<VMString*>(rightObj);

    std::ifstream file(fileName->GetStdString(), std::ifstream::in);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();

        VMString* result = Universe::NewString(buffer.str());
        return result;
    } else {
        return load_ptr(nilObject);
    }
}

void _System::PrintStackTrace(Interpreter*, VMFrame* frame) {
    frame->PrintStackTrace();
}

_System::_System(void) : PrimitiveContainer() {
    gettimeofday(&start_time, nullptr);

    Add("global:", &sysGlobal_, false);
    Add("global:put:", &sysGlobalPut, false);
    Add("hasGlobal:", &sysHasGlobal_, false);
    Add("load:", &sysLoad_, false);
    Add("exit:", &sysExit_, false);
    Add("printString:", &sysPrintString_, false);
    Add("printNewline", &sysPrintNewline, false);
    Add("errorPrint:", &sysErrorPrint_, false);
    Add("errorPrintln:", &sysErrorPrintNewline_, false);
    Add("time", &sysTime, false);
    Add("ticks", &sysTicks, false);
    Add("fullGC", &sysFullGC, false);

    Add("loadFile:", &sysLoadFile_, false);
    SetPrimitive("printStackTrace",
                 new Routine<_System>(this, &_System::PrintStackTrace, false));
}

_System::~_System() {}
