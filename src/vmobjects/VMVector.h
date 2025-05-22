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
    // cout<<"Last: "<<to_string(last)<<endl;
    VMArray* storage = load_ptr(this->storage);
    // cout<<"Sub-array-fields:"<<storage->GetNumberOfIndexableFields()<<endl;

    // cout<<"THIS BIT RUNNING -1"<<endl;
    // Check if we need to expand capacity
    if ((last) >= storage->GetNumberOfIndexableFields()) {
        // cout<<"THIS BIT RUNNING 1"<<endl;
        VMArray* newStorage = storage->CopyAndExtendWith(value);
        // cout<<"THIS BIT RUNNING 2"<<endl;
        this->storage = store_with_separate_barrier(newStorage);
        // cout<<"THIS BIT RUNNING 3"<<endl;
        write_barrier(this, this->storage);
        // cout<<"THIS BIT RUNNING 4"<<endl;

    } else { // No need to expand
        // cout<<"THIS BIT RUNNING *1"<<endl;
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
