/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.img.h
*  PURPOSE:     IMG R* Games archive management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_
#define _FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_

// Interface to handle compression of IMG archive (for XBOX Vice City and III)
struct CIMGArchiveCompressionHandler abstract
{
    virtual bool        IsStreamCompressed( CFile *stream ) const = 0;

    virtual bool        Decompress( CFile *inputStream, CFile *outputStream ) = 0;
    virtual bool        Compress( CFile *inputStream, CFile *outputStream ) = 0;
};

class CIMGArchiveTranslatorHandle abstract : public CArchiveTranslator
{
public:
    // Special functions just available for IMG archives.
    virtual void        SetCompressionHandler( CIMGArchiveCompressionHandler *handler ) = 0;
};

#endif //_FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_