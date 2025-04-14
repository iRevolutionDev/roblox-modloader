/**
  @file MemoryManager.cpp

  @maintainer Morgan McGuire, http://graphics.cs.williams.edu
  @created 2009-04-20
  @edited  2009-05-29

  Copyright 2000-2009, Morgan McGuire.
  All rights reserved.
 */

#include "RobloxModLoader/roblox/g3d/MemoryManager.h"
#include "RobloxModLoader/roblox/g3d/System.h"

namespace G3D {

MemoryManager::MemoryManager() {}


void* MemoryManager::alloc(size_t s) {
    return System::malloc(s);
}


void MemoryManager::free(void* ptr) {
    System::free(ptr);
}


bool MemoryManager::isThreadsafe() const {
    return true;
}


MemoryManager* MemoryManager::create() {
    static MemoryManager m;
    return &m;
}


///////////////////////////////////////////////////

}
