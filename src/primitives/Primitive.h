#pragma once

#include "../primitivesCore/PrimitiveContainer.h"
#include "../vmobjects/ObjectFormats.h"

class _Primitive : public PrimitiveContainer {
public:
    _Primitive();

    void InvokeOn_With_(VMFrame*);
};
