/**
  @file MemoryManager.h

  @maintainer Morgan McGuire, http://graphics.cs.williams.edu
  @created 2009-04-20
  @edited  2009-04-20

  Copyright 2000-2009, Morgan McGuire.
  All rights reserved.
 */
#ifndef G3D_MemoryManager_h
#define G3D_MemoryManager_h

#include <stdlib.h>
#include "platform.h"

namespace G3D {

/** 
   Abstraction of memory management.
   Default implementation uses G3D::System::malloc and is threadsafe.

   \sa CRTMemoryManager, AlignedMemoryManager, AreaMemoryManager */
class MemoryManager {
protected:

    MemoryManager();

public:

    typedef MemoryManager* Ref;

    /** Return a pointer to \a s bytes of memory that are unused by
        the rest of the program.  The contents of the memory are
        undefined */
    virtual void* alloc(size_t s);

    /** Invoke to declare that this memory will no longer be used by
        the program.  The memory manager is not required to actually
        reuse or release this memory. */
    virtual void free(void* ptr);

    /** Returns true if this memory manager is threadsafe (i.e., alloc
        and free can be called asychronously) */
    virtual bool isThreadsafe() const;

    /** Return the instance. There's only one instance of the default
        MemoryManager; it is cached after the first creation. */
    static MemoryManager* create();
};

}

#endif
