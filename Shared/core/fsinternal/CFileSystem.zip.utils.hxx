/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.utils.hxx
*  PURPOSE:     ZIP archive filesystem internal .cpp header
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _ZIP_INTERNAL_CPP_HEADER_
#define _ZIP_INTERNAL_CPP_HEADER_

// Structures that are used by the .zip extension code.
struct zip_mapped_rdircheck
{
    inline bool Perform( const char map[1024], size_t count, long& off ) const
    {
        unsigned short n = (unsigned int)count + 1;
        const char *cur = map + n;

        while ( n )
        {
            if ( *(unsigned int*)--cur == ZIP_SIGNATURE )
            {
                off = n + 3;
                return true;
            }

            n--;
        }

        return false;
    }
};

#pragma pack(1)

struct _endDir
{
    unsigned short  diskID;
    unsigned short  diskAlign;
    unsigned short  entries;
    unsigned short  totalEntries;
    size_t          centralDirectorySize;
    size_t          centralDirectoryOffset;

    unsigned short  commentLen;
};

#pragma pack()

#endif //_ZIP_INTERNAL_CPP_HEADER_