#ifndef AINLINE
#ifdef _MSC_VER
#define AINLINE __forceinline
#elif __linux__
#define AINLINE __attribute__((always_inline))
#else
#define AINLINE
#endif
#endif

#include <MemoryUtils.h>

#include "renderware.h"

#pragma warning(push)
#pragma warning(disable: 4290)

// Global allocator
extern void* operator new( size_t memSize ) throw(std::bad_alloc);
extern void* operator new( size_t memSize, const std::nothrow_t nothrow ) throw();
extern void* operator new[]( size_t memSize ) throw(std::bad_alloc);
extern void* operator new[]( size_t memSize, const std::nothrow_t nothrow ) throw();
extern void operator delete( void *ptr ) throw();
extern void operator delete[]( void *ptr ) throw();

#pragma warning(pop)

template <typename keyType, typename valueType>
struct uniqueMap_t
{
    struct pair
    {
        keyType key;
        valueType value;
    };

    typedef std::vector <pair> list_t;

    list_t pairs;

    inline valueType& operator [] ( const keyType& checkKey )
    {
        pair *targetIter = NULL;

        for ( list_t::iterator iter = pairs.begin(); iter != pairs.end(); iter++ )
        {
            if ( iter->key == checkKey )
            {
                targetIter = &*iter;
            }
        }

        if ( targetIter == NULL )
        {
            pair pair;
            pair.key = checkKey;

            pairs.push_back( pair );

            targetIter = &pairs.back();
        }

        return targetIter->value;
    }
};