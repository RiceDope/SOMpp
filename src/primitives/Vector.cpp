#include "Vector.h"
#include <cstdint>

#include "../vm/Universe.h"
#include "../vmobjects/ObjectFormats.h"
#include "../vmobjects/VMVector.h"
#include "../vmobjects/VMFrame.h"

static vm_oop_t vecNew(vm_oop_t, vm_oop_t arg) {
    int64_t const size = INT_VAL(arg);
    cout << "Running the primitive for creating a new vector";
    return Universe::NewVector(size);
}

_Vector::_Vector() {
    Add("new:", &vecNew, true);
}