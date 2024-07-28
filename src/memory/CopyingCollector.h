#pragma once

#include "GarbageCollector.h"

class CopyingHeap;

class CopyingCollector: public GarbageCollector<CopyingHeap> {
public:
    explicit CopyingCollector(CopyingHeap* h) : GarbageCollector(h) {};
private:
    void Collect() override;
};
