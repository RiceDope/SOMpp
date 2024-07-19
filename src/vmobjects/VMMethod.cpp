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

#include <cstdint>
#include <cstring>
#include <string>

#include "../interpreter/Interpreter.h"
#include "../memory/Heap.h"
#include "../misc/defs.h"
#include "../vm/Globals.h"
#include "../vm/Universe.h"  // // NOLINT(misc-include-cleaner) it's required to make the types complete
#include "ObjectFormats.h"
#include "Signature.h"
#include "VMClass.h"
#include "VMFrame.h"
#include "VMMethod.h"
#include "VMObject.h"
#include "VMSymbol.h"

#ifdef UNSAFE_FRAME_OPTIMIZATION
const long VMMethod::VMMethodNumberOfFields = 8;
#else
const long VMMethod::VMMethodNumberOfFields = 7;
#endif

VMMethod::VMMethod(long bcCount, long numberOfConstants) :
        VMInvokable(VMMethodNumberOfFields) {
#ifdef UNSAFE_FRAME_OPTIMIZATION
    cachedFrame = nullptr;
#endif
    store_ptr(bcLength, NEW_INT(bcCount));
    store_ptr(numberOfLocals, NEW_INT(0));
    store_ptr(maximumNumberOfStackElements, NEW_INT(0));
    store_ptr(numberOfArguments, NEW_INT(0));
    store_ptr(this->numberOfConstants, NEW_INT(numberOfConstants));

    indexableFields = (gc_oop_t*)(&indexableFields + 2);
    for (long i = 0; i < numberOfConstants; ++i) {
        indexableFields[i] = nilObject;
    }
    bytecodes = (uint8_t*)(&indexableFields + 2 + GetNumberOfIndexableFields());
}

VMMethod* VMMethod::Clone() const {
    VMMethod* clone = new (GetHeap<HEAP_CLS>(), GetObjectSize() - sizeof(VMMethod) ALLOC_MATURE) VMMethod(*this);
    memcpy(SHIFTED_PTR(clone, sizeof(VMObject)), SHIFTED_PTR(this,
                    sizeof(VMObject)), GetObjectSize() -
            sizeof(VMObject));
    clone->indexableFields = (gc_oop_t*)(&(clone->indexableFields) + 2);
    clone->bytecodes = (uint8_t*)(&(clone->indexableFields) + 2 + GetNumberOfIndexableFields());
    return clone;
}

void VMMethod::SetSignature(VMSymbol* sig) {
    VMInvokable::SetSignature(sig);
    SetNumberOfArguments(Signature::GetNumberOfArguments(sig));
}

void VMMethod::WalkObjects(walk_heap_fn walk) {
    VMInvokable::WalkObjects(walk);

    numberOfLocals    = walk(numberOfLocals);
    maximumNumberOfStackElements = walk(maximumNumberOfStackElements);
    bcLength          = walk(bcLength);
    numberOfArguments = walk(numberOfArguments);
    numberOfConstants = walk(numberOfConstants);
#ifdef UNSAFE_FRAME_OPTIMIZATION
    if (cachedFrame != nullptr)
        cachedFrame = static_cast<VMFrame*>(walk(cachedFrame));
#endif

    long numIndexableFields = GetNumberOfIndexableFields();
    for (long i = 0; i < numIndexableFields; ++i) {
        if (indexableFields[i] != nullptr) {
            indexableFields[i] = walk(indexableFields[i]);
        }
    }
}

#ifdef UNSAFE_FRAME_OPTIMIZATION
VMFrame* VMMethod::GetCachedFrame() const {
    return cachedFrame;
}

void VMMethod::SetCachedFrame(VMFrame* frame) {
    cachedFrame = frame;
    if (frame != nullptr) {
        frame->SetContext(nullptr);
        frame->SetBytecodeIndex(0);
        frame->ResetStackPointer();
        write_barrier(this, cachedFrame);
    }
}
#endif

void VMMethod::SetNumberOfLocals(long nol) {
    store_ptr(numberOfLocals, NEW_INT(nol));
}

long VMMethod::GetMaximumNumberOfStackElements() const {
    return INT_VAL(load_ptr(maximumNumberOfStackElements));
}

void VMMethod::SetMaximumNumberOfStackElements(long stel) {
    store_ptr(maximumNumberOfStackElements, NEW_INT(stel));
}

void VMMethod::SetNumberOfArguments(long noa) {
    store_ptr(numberOfArguments, NEW_INT(noa));
}

long VMMethod::GetNumberOfBytecodes() const {
    return INT_VAL(load_ptr(bcLength));
}

void VMMethod::Invoke(Interpreter* interp, VMFrame* frame) {
    VMFrame* frm = interp->PushNewFrame(this);
    frm->CopyArgumentsFrom(frame);
}

void VMMethod::SetHolder(VMClass *hld) {
    VMInvokable::SetHolder(hld);
    SetHolderAll(hld);
}

void VMMethod::SetHolderAll(VMClass* hld) {
    long numIndexableFields = GetNumberOfIndexableFields();
    for (long i = 0; i < numIndexableFields; ++i) {
        vm_oop_t o = GetIndexableField(i);
        if (!IS_TAGGED(o)) {
            VMInvokable* vmi = dynamic_cast<VMInvokable*>(AS_OBJ(o));
            if (vmi != nullptr) {
                vmi->SetHolder(hld);
            }
        }
    }
}

std::string VMMethod::AsDebugString() const {
    VMClass* holder = GetHolder();
    std::string holder_str;
    if (holder == load_ptr(nilObject)) {
        holder_str = "nil";
    } else {
        holder_str = holder->GetName()->GetStdString();
    }
    return "Method(" + holder_str + ">>#" + GetSignature()->GetStdString() + ")";
}
