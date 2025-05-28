#pragma once

#include <cstddef>

#include "../vm/Universe.h"
#include "../vmobjects/VMInteger.h"
#include "../vmobjects/VMObject.h"
#include "ObjectFormats.h"
#include "VMArray.h"

class VMVector : public VMObject {
public:
    typedef GCVector Stored;

    explicit VMVector(vm_oop_t first, vm_oop_t last, VMArray* storage);

    /* handles 1 - 0 indexing*/
    [[nodiscard]] inline vm_oop_t GetIndexableField(size_t index) {
        int64_t const first = INT_VAL(load_ptr(this->first));
        int64_t const last = INT_VAL(load_ptr(this->last));
        VMArray* const storage = load_ptr(this->storage);

        if (index < 1 || index > last - first) { // Check this error handling again
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
        vm_oop_t returned = GetIndexableField(last - 1);
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

        if (last >=
            storage->GetNumberOfIndexableFields()) {  // Expand the array
            // Expand by the correct amount *2 by default in the native som
            // implementation
            VMArray* newStorage =
                Universe::NewArray(storage->GetNumberOfIndexableFields() * 2);

            storage->CopyIndexableFieldsTo(newStorage);
            newStorage->SetIndexableField(last - 1, value);

            SetField(2 /* storage */, newStorage);

        } else {  // Just set the new value
            storage->SetIndexableField(last - 1, value);
        }

        last += 1;
        this->last = store_ptr(this->last, NEW_INT(last));
    }

    inline vm_oop_t RemoveLast() {
        int64_t last = INT_VAL(load_ptr(this->last));
        if (last <= 1) {
            ErrorExit("Cannot remove last element from an empty vector.");
        }
        return Remove(NEW_INT(last-1-1));
    }

    inline vm_oop_t RemoveFirst() {
        // This method will just increment the first index
        int64_t first = INT_VAL(load_ptr(this->first));
        if (first >= INT_VAL(load_ptr(this->last))) {
            ErrorExit("Cannot remove first element from an empty vector.");
        }
        vm_oop_t itemToRemove = GetIndexableField(1); // This is 1 because GetIndexableField handles 1 to 0 indexing
        first += 1;  // Increment the first index
        this->first = store_ptr(this->first, NEW_INT(first));
        return itemToRemove;
    }

    /* Return the index if object is located, else return -1 for not found*/
    inline vm_oop_t IndexOf(vm_oop_t other) {
        int64_t first = INT_VAL(load_ptr(this->first));
        int64_t last = INT_VAL(load_ptr(this->last));
        VMArray* storage = load_ptr(this->storage);

        AbstractVMObject* otherObj = AS_OBJ(other);

        // Iterate through from first-1 (inclusive) to last-1 (Not inclusive)
        for (int64_t i = first-1; i < last - 1; ++i) {
            vm_oop_t current = storage->GetIndexableField(i);

            // Check where integers are tagged or references can be checked
            if (current == other) {
                return NEW_INT(i - first + 2);
            }

            // Check where internal values matter i.e. Strings
            if (!IS_TAGGED(current) && !IS_TAGGED(other)) {
                AbstractVMObject* currentObj = AS_OBJ(current);

                if (currentObj->AsDebugString() == otherObj->AsDebugString()) {
                    return NEW_INT(i - first + 2);
                }
            }

        }
        return NEW_INT(-1);
    }

    [[nodiscard]] inline vm_oop_t contains(vm_oop_t other) {
        vm_oop_t where = IndexOf(other);
        if (INT_VAL(where) < 0) {
            return load_ptr(falseObject);
        } else {
            return load_ptr(trueObject);
        }
    }

    [[nodiscard]] inline vm_oop_t Remove(vm_oop_t inx) {
        int64_t first = INT_VAL(load_ptr(this->first));
        int64_t last = INT_VAL(load_ptr(this->last));
        VMArray* storage = load_ptr(this->storage);
        int64_t index = INT_VAL(inx);

        if (index < 0 || index > last - first) {
            IndexOutOfBounds(index, (last - first));
        }

        vm_oop_t itemToRemove = GetIndexableField(last-1);

        // Shift all elements after the index to the left
        for (size_t i = index; i < last - first; ++i) {
            storage->SetIndexableField(first + i - 1,
                                       storage->GetIndexableField(first + i));
        }

        last -= 1;
        this->last = store_ptr(this->last, NEW_INT(last));

        return itemToRemove;
    }

    static __attribute__((noreturn)) __attribute__((noinline)) void
    IndexOutOfBounds(size_t idx, size_t size);

private:
    static const size_t VMVectorNumberOfFields;

    gc_oop_t first;
    gc_oop_t last;
    GCArray* storage;
};
