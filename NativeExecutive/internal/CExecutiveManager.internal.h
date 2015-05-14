#ifndef _NATIVE_EXECUTIVE_MANAGER_INTERNAL_
#define _NATIVE_EXECUTIVE_MANAGER_INTERNAL_

// Memory allocator used in this environment.
struct nativeExecutiveAllocator
{
    inline void* Allocate( size_t memSize )
    {
        return malloc( memSize );
    }

    inline void Free( void *memPtr, size_t memSize )
    {
        return free( memPtr );
    }
};

namespace ExecutiveManager
{
    extern nativeExecutiveAllocator moduleAllocator;
};

class CExecutiveManagerNative : public CExecutiveManager
{
public:
    inline CExecutiveManagerNative( void ) : CExecutiveManager()
    {
        return;
    }

    inline ~CExecutiveManagerNative( void )
    {
        return;
    }

    // This is the native representation of the executive manager.
    // Every alive type of CExecutiveManager should be of this type.
};

typedef StaticPluginClassFactory <CExecutiveManagerNative> executiveManagerFactory_t;

extern executiveManagerFactory_t executiveManagerFactory;

#endif //_NATIVE_EXECUTIVE_MANAGER_INTERNAL_