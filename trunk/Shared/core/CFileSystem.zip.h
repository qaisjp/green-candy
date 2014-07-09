/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.h
*  PURPOSE:     ZIP archive filesystem
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_ZIP_
#define _FILESYSTEM_ZIP_

// ZIP extension struct.
struct zipExtension
{
    void                        Init            ( void );
    void                        Shutdown        ( void );

    CArchiveTranslator*         NewArchive      ( CFile& writeStream );
    CArchiveTranslator*         OpenArchive     ( CFile& readWriteStream );

    // Private extension methods.
    CFileTranslator*            GetTempRoot     ( void );

    // Extension members.
    // ... for managing temporary files (OS dependent).
    // See zipExtension::Init().
    CFileTranslator*            sysTmp;
    CFileTranslator*            sysTmpRoot;
};

#endif //_FILESYSTEM_ZIP_
