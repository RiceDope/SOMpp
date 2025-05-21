#pragma once

#include <cstddef>

#include "../vmobjects/VMInteger.h"
#include "../vmobjects/VMObject.h"

class VMVector : public VMObject {
public:
    typedef GCVector Stored;

    explicit VMVector(size_t vectorSize, size_t additionalBytes)
        : VMObject(vectorSize +
                      0 /* VMVector is not allowed to have any fields itself */,
                  additionalBytes + sizeof(VMVector)) {
        assert(VMVectorNumberOfFields == 0);
    }

    private:
        static const size_t VMVectorNumberOfFields;
};