/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.cpp
*  PURPOSE:     ZIP archive filesystem
*
*  Docu: https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <sstream>

// Include internal (private) definitions.
#include "fsinternal/CFileSystem.internal.h"
#include "fsinternal/CFileSystem.zip.internal.h"

extern CFileSystem *fileSystem;

#include "CFileSystem.Utils.hxx"
#include "fsinternal/CFileSystem.zip.utils.hxx"


CArchiveTranslator* zipExtension::NewArchive( CFile& writeStream )
{
    if ( !writeStream.IsWriteable() )
        return NULL;

    return new CZIPArchiveTranslator( *this, writeStream );
}

CArchiveTranslator* zipExtension::OpenArchive( CFile& readWriteStream )
{
    {
        zip_mapped_rdircheck checker;

        if ( !FileSystem::MappedReaderReverse <char [1024]>( readWriteStream, checker ) )
            return NULL;
    }

    _endDir dirEnd;

    if ( !readWriteStream.ReadStruct( dirEnd ) )
        return NULL;

    readWriteStream.Seek( -(long)dirEnd.centralDirectorySize - (long)sizeof( dirEnd ) - 4, SEEK_CUR );

    CZIPArchiveTranslator *zip = new CZIPArchiveTranslator( *this, readWriteStream );

    try
    {
        zip->ReadFiles( dirEnd.entries );
    }
    catch( ... )
    {
        delete zip;

        return NULL;
    }

    return zip;
}

bool _File_CreateDirectory( const char *osPath )
{
#ifdef __linux__
    if ( mkdir( osPath, FILE_ACCESS_FLAG ) == 0 )
        return true;

    switch( errno )
    {
    case EEXIST:
    case 0:
        return true;
    }

    return false;
#elif defined(_WIN32)
    return CreateDirectory( osPath, NULL ) != FALSE;
#else
    return false;
#endif //OS DEPENDANT CODE
}

CFileTranslator* zipExtension::GetTempRoot( void )
{
    if ( sysTmpRoot )
        return sysTmpRoot;

    filePath tmpDir;

    if ( !sysTmp )
    {
#ifdef _WIN32
        char buf[1024];

        GetTempPath( sizeof( buf ), buf );
        tmpDir = buf;
        tmpDir += '/';
#elif defined(__linux__)
        const char *dir = getenv("TEMPDIR");

        if ( !dir )
            tmpDir = "/tmp";
        else
            tmpDir = dir;

        tmpDir += '/';

        // On linux we cannot be sure that our directory exists.
        if ( !_File_CreateDirectory( tmpDir.c_str() ) )
            exit( 7098 );
#endif //OS DEPENDANT CODE

        sysTmp = fileSystem->CreateTranslator( tmpDir.c_str() );

        if ( !sysTmp )
            return NULL;
    }
    else
    {
        bool success = sysTmp->GetFullPath( "@", false, tmpDir );

        if ( !success )
            return NULL;
    }

    // Generate a random sub-directory inside of the global OS temp directory.
    {
        std::stringstream stream;

        stream.precision( 0 );
        stream << ( rand() % 647251833 );

        tmpDir += "&$!reAr";
        tmpDir += stream.str();
        tmpDir += "_/";
    }

    // Make sure the temporary directory exists.
    bool creationSuccessful = _File_CreateDirectory( tmpDir.c_str() );

    if ( creationSuccessful )
    {
        // Create the .zip temporary root
        sysTmpRoot = fileSystem->CreateTranslator( tmpDir.c_str() );
        return sysTmpRoot;
    }

    return NULL;
}

void zipExtension::Init( void )
{
    // We do not have to create our private directory if we do not require it.
    sysTmp = NULL;
    sysTmpRoot = NULL;
}

void zipExtension::Shutdown( void )
{
    if ( sysTmpRoot )
    {
        filePath tmpDir;

        if ( sysTmp )
        {
            sysTmpRoot->GetFullPath( "@", false, tmpDir );
        }

        delete sysTmpRoot;

        sysTmpRoot = NULL;

        if ( sysTmp )
        {
            sysTmp->Delete( tmpDir.c_str() );
        }
    }

    if ( sysTmp )
    {
        delete sysTmp;

        sysTmp = NULL;
    }
}

// Export methods into the CFileSystem class directly.
CArchiveTranslator* CFileSystem::CreateZIPArchive( CFile& file )
{
    return m_zipExtension.NewArchive( file );
}

CArchiveTranslator* CFileSystem::OpenArchive( CFile& file )
{
    return m_zipExtension.OpenArchive( file );
}