#ifndef AINLINE
#ifdef _MSC_VER
#define AINLINE __forceinline
#elif __linux__
#define AINLINE __attribute__((always_inline))
#else
#define AINLINE
#endif
#endif

// Macro that defines how alignment works.
//  num: base of the number to be aligned
//  sector: aligned-offset that should be added to num
//  align: number of bytes to align to
// EXAMPLE: ALIGN( 0x1001, 4, 4 ) -> 0x1000 (equivalent of compiler structure padding alignment)
//          ALIGN( 0x1003, 1, 4 ) -> 0x1000
//          ALIGN( 0x1003, 2, 4 ) -> 0x1004
template <typename numberType>
AINLINE numberType _ALIGN_GP( numberType num, numberType sector, numberType align )
{
	// General purpose alignment routine.
    // Not as fast as the bitfield version.
    numberType sectorOffset = ((num) + (sector) - 1);

    return sectorOffset - ( sectorOffset % align );
}

template <typename numberType>
AINLINE numberType _ALIGN_NATIVE( numberType num, numberType sector, numberType align )
{
	const size_t bitCount = sizeof( align ) * 8;

    // assume math based on x86 bits.
    if ( std::bitset <bitCount> ( align ).count() == 1 )
    {
        //bitfield version. not compatible with non-bitfield alignments.
        return (((num) + (sector) - 1) & (~((align) - 1)));
    }
    else
    {
		return _ALIGN_GP( num, sector, align );
    }
}

template <typename numberType>
AINLINE numberType ALIGN( numberType num, numberType sector, numberType align )
{
	return _ALIGN_GP( num, sector, align );
}

// Optimized primitives.
template <> AINLINE char			ALIGN( char num, char sector, char align )								{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned char	ALIGN( unsigned char num, unsigned char sector, unsigned char align )	{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE short			ALIGN( short num, short sector, short align )							{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned short	ALIGN( unsigned short num, unsigned short sector, unsigned short align ){ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE int				ALIGN( int num, int sector, int align )									{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned int	ALIGN( unsigned int num, unsigned int sector, unsigned int align )
{
	return (unsigned int)_ALIGN_NATIVE( (int)num, (int)sector, (int)align );
}

// Helper macro (equivalent of EXAMPLE 1)
template <typename numberType>
inline numberType ALIGN_SIZE( numberType num, numberType sector )
{
    return ( ALIGN( (num), (sector), (sector) ) );
}

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