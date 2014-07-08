/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.common.h
*  PURPOSE:     Common definitions used by the FileSystem module
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_COMMON_DEFINITIONS_
#define _FILESYSTEM_COMMON_DEFINITIONS_

// Mathematically correct data slice logic.
// It is one of the most important theorems in computing abstraction.
template <typename numberType>
class sliceOfData
{
public:
    inline sliceOfData( numberType startOffset, numberType dataSize )
    {
        this->startOffset = startOffset;
        this->endOffset = startOffset + ( dataSize - 1 );
    }

    enum eIntersectionResult
    {
        INTERSECT_EQUAL,
        INTERSECT_INSIDE,
        INTERSECT_BORDER_START,
        INTERSECT_BORDER_END,
        INTERSECT_ENCLOSING,
        INTERSECT_FLOATING_START,
        INTERSECT_FLOATING_END,
        INTERSECT_UNKNOWN   // if something went horribly wrong (like NaNs).
    };

    // Static methods for easier result management.
    static bool isBorderIntersect( eIntersectionResult result )
    {
        return ( result == INTERSECT_BORDER_START || result == INTERSECT_BORDER_END );
    }

    static bool isFloatingIntersect( eIntersectionResult result )
    {
        return ( result == INTERSECT_FLOATING_START || result == INTERSECT_FLOATING_END );
    }

    inline numberType GetSliceStartPoint( void ) const
    {
        return startOffset;
    }

    inline numberType GetSliceEndPoint( void ) const
    {
        return endOffset;
    }

    inline eIntersectionResult intersectWith( const sliceOfData& right ) const
    {
        // Make sure the slice has a valid size.
        if ( this->endOffset >= this->startOffset &&
             right.endOffset >= right.startOffset )
        {
            // Get generic stuff.
            numberType sliceStartA = startOffset, sliceEndA = endOffset;
            numberType sliceStartB = right.startOffset, sliceEndB = right.endOffset;

            // Handle all cases.
            // We only implement the logic with comparisons only, as it is the most transparent for all number types.
            if ( sliceStartA == sliceStartB && sliceEndA == sliceEndB )
            {
                return INTERSECT_EQUAL;
            }

            if ( sliceStartB >= sliceStartA && sliceEndB <= sliceEndA )
            {
                return INTERSECT_ENCLOSING;
            }

            if ( sliceStartB <= sliceStartA && sliceEndB >= sliceEndA )
            {
                return INTERSECT_INSIDE;
            }

            if ( sliceStartB < sliceStartA && ( sliceEndB >= sliceStartA && sliceEndB <= sliceEndA ) )
            {
                return INTERSECT_BORDER_START;
            }

            if ( sliceEndB > sliceEndA && ( sliceStartB >= sliceStartA && sliceStartB <= sliceEndA ) )
            {
                return INTERSECT_BORDER_END;
            }

            if ( sliceStartB < sliceStartA && sliceEndB < sliceStartA )
            {
                return INTERSECT_FLOATING_END;
            }

            if ( sliceStartB > sliceEndA && sliceEndB > sliceEndA )
            {
                return INTERSECT_FLOATING_START;
            }
        }

        return INTERSECT_UNKNOWN;
    }

private:
    numberType startOffset;
    numberType endOffset;
};

#endif //_FILESYSTEM_COMMON_DEFINITIONS_