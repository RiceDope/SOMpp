#pragma once

#include <cstddef>

#include "../vmobjects/VMInteger.h"
#include "../vmobjects/VMObject.h"
#include "../vm/Universe.h"
#include "VMArray.h"
#include "ObjectFormats.h"

class VMVector : public VMObject {
public:
    typedef GCVector Stored;

    explicit VMVector(vm_oop_t first, vm_oop_t last, VMArray* storage);

    [[nodiscard]] inline vm_oop_t GetIndexableField(size_t index) {
        int64_t const first = INT_VAL(load_ptr(this->first));
        int64_t const last = INT_VAL(load_ptr(this->last));
        VMArray* const storage = load_ptr(this->storage);
        if (index < 1 || index > first + last) {
            IndexOutOfBounds(index, (last - first));
            // TODO(smarr): check if this would be correct
        }
        vm_oop_t returned = storage->GetIndexableField(first + index - 2);
        return returned;
    }

    inline void SetIndexableField(size_t index, vm_oop_t value) {
        int64_t const first = INT_VAL(load_ptr(this->first));
        int64_t const last = INT_VAL(load_ptr(this->last));
        VMArray* const storage = load_ptr(this->storage);
        if (index < 1 || index > first + last) {
            IndexOutOfBounds(index, (last - first));
            // TODO(smarr): check if this would be correct
        }
        storage->SetIndexableField(first + index - 2, value);
    }

    inline void Append(vm_oop_t value) {

    int64_t first = INT_VAL(load_ptr(this->first));
    int64_t last = INT_VAL(load_ptr(this->last));
    VMArray* storage = load_ptr(this->storage);

    // Check if we need to expand capacity
    if ((last) >= storage->GetNumberOfIndexableFields()) {
        VMArray* newStorage = storage->CopyAndExtendWith(value);
        this->storage = store_with_separate_barrier(newStorage);
        write_barrier(this, this->storage);

    } else { // No need to expand
        storage->SetIndexableField(last, value);
        write_barrier(storage, value);
    }

    last = last + 1;
    this->last = store_ptr(this->last, NEW_INT(last));

}


    static __attribute__((noreturn)) __attribute__((noinline)) void
    IndexOutOfBounds(size_t idx, size_t size);

private:
    static const size_t VMVectorNumberOfFields;

    gc_oop_t first;
    gc_oop_t last;
    GCArray* storage;
};
