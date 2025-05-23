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

    [[nodiscard]] inline vm_oop_t GetFirst() {
        int64_t first = INT_VAL(load_ptr(this->first));
        vm_oop_t returned = GetIndexableField(1);
        return returned;
    }

    [[nodiscard]] inline vm_oop_t GetLast() {
        int64_t last = INT_VAL(load_ptr(this->last));
        int64_t first = INT_VAL(load_ptr(this->first));
        vm_oop_t returned = GetIndexableField(last);
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

        if (last >= storage->GetNumberOfIndexableFields()) {
            cout<<"RUNNING THIS"<<endl;
            VMArray* newStorage = storage->CopyAndExtendWith(value);
            SetField(2 /* storage */, newStorage);

        } else {
            storage->SetIndexableField(last-1, value);
        }

        last += 1;
        this->last = store_ptr(this->last, NEW_INT(last));

        cout<<"SPACE: "<<storage->GetNumberOfIndexableFields()<<endl;
        cout<<"LAST: "<<last<<endl;
    }


    static __attribute__((noreturn)) __attribute__((noinline)) void
    IndexOutOfBounds(size_t idx, size_t size);

private:
    static const size_t VMVectorNumberOfFields;

    gc_oop_t first;
    gc_oop_t last;
    GCArray* storage;
};
