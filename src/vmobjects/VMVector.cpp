#include "../vmobjects/VMVector.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

#include "../memory/Heap.h"
#include "../misc/defs.h"
#include "../vm/Print.h"
#include "../vm/Universe.h"
#include "../vmobjects/ObjectFormats.h"
#include "../vmobjects/VMObject.h"

const size_t VMVector::VMVectorNumberOfFields = 0;