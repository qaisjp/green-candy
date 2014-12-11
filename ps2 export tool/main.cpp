#include "StdInc.h"

#include <renderware.h>
#include <fstream>

#include <SharedUtil.hpp>
#include <iostream>
#include <streambuf>

static CFileSystem *fsHandle = NULL;

struct _exportFileStruct
{
    CFileTranslator *targetRoot;
    CFileTranslator *sourceRoot;
    CFileTranslator *archiveRoot;
};

static void SaveTexture( rw::NativeTexture& tex, const char *absPath, const char *txdName )
{
    if ( tex.platformData->getPaletteType() != rw::PALETTE_4BIT )
    {
        //tex.convertToFormat(rw::RASTER_8888);
        tex.convertToPalette(rw::PALETTE_4BIT);
        tex.writeTGA( absPath );
    }
}

static bool ProcessTXDArchive( CFileTranslator *srcRoot, CFile *srcStream, CFile *targetStream, CFileTranslator *debugOutputRoot )
{
    bool hasProcessed = false;

    // Optimize the texture archive.
    FileSystem::fileStreamBuf buf( srcStream );
    std::istream rw( &buf );

    rw::TextureDictionary txd;
    
    if ( rw.good() )
    {
        txd.read(rw);
    }

    const filePath& texPath = srcStream->GetPath();

    bool canOutputDebug = false;

    filePath fileNameItem;

    bool hasDebugName = srcRoot->GetRelativePathFromRoot( texPath.c_str(), true, fileNameItem );

    if ( hasDebugName && debugOutputRoot )
    {
        // Create a directory to save textures in (in TGA format).
        std::string dirName( "conv_tga/" );

        bool createDirectorySuccess = debugOutputRoot->CreateDir( dirName.c_str() );

        if ( createDirectorySuccess )
        {
            canOutputDebug = true;
        }
    }

    // Process all textures.
    bool processSuccessful = true;

    for ( unsigned int n = 0; n < txd.texList.size(); n++ )
    {
        rw::NativeTexture& tex = txd.texList[n];

        bool isPrepared = false;

        if ( tex.platform == rw::PLATFORM_PS2 )
        {
            //tex.convertFromPS2( 0x40 );

            isPrepared = true;
        }

        // If the texture is prepared, do whatever.
        if ( isPrepared )
        {
            // Palettize the texture and convert it back into PS2 format.
            //tex.convertToPalette( rw::RASTER_PAL4 );
            //tex.convertToPS2();

            if ( canOutputDebug )
            {
                // Create a path to store the textures to.
                std::string textureSaveName( fileNameItem );
                textureSaveName += "_";
                textureSaveName += tex.name;
                textureSaveName += ".tga";

                filePath absTexPath;

                std::string browsePath = textureSaveName;

                bool hasAbsTexPath = debugOutputRoot->GetFullPath( browsePath.c_str(), true, absTexPath );

                if ( hasAbsTexPath )
                {
                    SaveTexture( tex, absTexPath.c_str(), fileNameItem.c_str() );
                }
            }
        }
    }

    // We do not perform any error checking for now.
    if ( processSuccessful )
    {
        // Write the TXD into the target stream.
        FileSystem::fileStreamBuf writebuf( targetStream );
        std::ostream rwout( &writebuf );

        if ( rwout.good() )
        {
            txd.write(rwout);

            hasProcessed = true;
        }
    }

    return hasProcessed;
}

struct _discFileTraverse
{
    CFileTranslator *discHandle;
    CFileTranslator *buildRoot;
};

static void _discFileCallback( const filePath& discFilePathAbs, void *userdata )
{
    _discFileTraverse *info = (_discFileTraverse*)userdata;

    // Do special logic for certain files.
    // Copy all files into the build root.
    CFile *sourceStream = NULL;
    {
        sourceStream = info->discHandle->Open( discFilePathAbs.c_str(), "rb" );
    }

    // Open the target stream.
    CFile *targetStream = NULL;
    {
        // Create a destination file inside of the build root using the relative path from the source trans root.
        filePath relPathFromRoot;
        
        bool hasRelativePath = info->discHandle->GetRelativePathFromRoot( discFilePathAbs.c_str(), true, relPathFromRoot );

        if ( hasRelativePath )
        {
            targetStream = info->buildRoot->Open( relPathFromRoot.c_str(), "wb" );
        }
    }

    if ( targetStream && sourceStream )
    {
        // Allow to perform custom logic on the source and target streams.
        bool hasCopiedFile = false;
        {
            SString fileName = FileSystem::GetFileNameItem( discFilePathAbs.c_str(), true, NULL );

            SString extention;

            bool hasExtension = SharedUtil::ExtractExtention( fileName, NULL, &extention );

            if ( hasExtension )
            {
                if ( extention.CompareI( "IMG" ) == true )
                {
                    printf( "processing %s ...\n", discFilePathAbs.c_str() );

                    // If we have found an IMG archive, we perform the same stuff for files inside of it.
                    // We copy the files into a new IMG archive tho.
                    CArchiveTranslator *newIMGRoot = fileSystem->CreateIMGArchive( *targetStream );

                    if ( newIMGRoot )
                    {
                        CArchiveTranslator *srcIMGRoot = fileSystem->OpenIMGArchive( *sourceStream );

                        if ( srcIMGRoot )
                        {
                            // Run into shit.
                            _discFileTraverse traverse;
                            traverse.discHandle = srcIMGRoot;
                            traverse.buildRoot = newIMGRoot;

                            srcIMGRoot->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );

                            printf( "writing... " );

                            newIMGRoot->Save();

                            printf( "done.\n" );

                            hasCopiedFile = true;
                            
                            delete srcIMGRoot;
                        }
                        else
                        {
                            printf( "not an IMG archive.\n" );
                        }

                        delete newIMGRoot;
                    }
                    else
                    {
                        printf( "failed to create new IMG archive for processing; defaulting to file-copy ...\n" );
                    }
                }
                else if ( extention.CompareI( "TXD" ) == true )
                {
                    printf( "optimizing texture container: %s ...", discFilePathAbs.c_str() );

                    bool couldProcessTXD = ProcessTXDArchive( info->discHandle, sourceStream, targetStream, NULL );

                    if ( couldProcessTXD )
                    {
                        hasCopiedFile = true;
                    }

                    printf( "OK\n" );
                }
            }
        }

        // If we have not yet created the new copy, we default to simple stream swap.
        if ( !hasCopiedFile && targetStream )
        {
            // Make sure we copy from the beginning of the source stream.
            sourceStream->Seek( 0, SEEK_SET );
            
            // Copy the stream contents.
            FileSystem::StreamCopy( *sourceStream, *targetStream );
        }
    }

    if ( targetStream )
    {
        delete targetStream;
    }

    if ( sourceStream )
    {
        delete sourceStream;
    }
}

inline uint8 randcolor( void )
{
    int random = rand();

    return (uint8)( ( ( (double)random) / RAND_MAX ) * 255.0f );
}

static void DebugFuncs( CFileTranslator *discHandle )
{
    return;

    // Debug a weird txd container...
    CFile *txdChat = discHandle->Open( "MODELS/GENERIC/VEHICLE.TXD", "rb" );

    if ( txdChat )
    {
        rw::TextureDictionary txd;

        FileSystem::fileStreamBuf buf( txdChat );
        std::istream rw( &buf );

        fsOffsetNumber_t origTxdEndPos = 0;

        if ( rw.good() )
        {
            txd.read( rw );

            // Check whether everything has been read.
            origTxdEndPos = txdChat->TellNative();
            fsOffsetNumber_t endPos = txdChat->GetSizeNative();

            if ( origTxdEndPos != endPos )
            {
                __asm nop
            }
        }

        const filePath& txdName = txdChat->GetPath();

        std::string nameItem = FileSystem::GetFileNameItem( txdName.c_str(), true, NULL );

        // Convert and write the texture.
        std::string pathToTXD = "txdout/" + nameItem;

        CFile *targetStream = fileRoot->Open( pathToTXD.c_str(), "wb" );

        if ( targetStream )
        {
            FileSystem::fileStreamBuf outbuf( targetStream );
            std::ostream rwout( &outbuf );

            if ( rwout.good() )
            {
                // Convert from PS2, convert to 4bit palette and convert back to PS2.
                for ( unsigned int n = 0; n < txd.texList.size(); n++ )
                {
                    rw::NativeTexture& tex = txd.texList.at( n );

                    // WARNING: conversion does destroy some platform native data that
                    // is embedded into the texture!
                    // We need a way to retrieve the data and set it back after conversion.

                    if ( tex.platform == rw::PLATFORM_PS2 )
                    {
                        tex.convertFromPS2( 0x40 );

                        // Write the texture somewhere.
                        std::string newName = std::string( "txdout/" ) + tex.name;
                        newName += ".tga";

                        //tex.convertToFormat( rw::RASTER_8888 );
                        if ( tex.platformData->getDepth() == 4 )
                        {
                            tex.convertToPalette( rw::PALETTE_8BIT );
                        }
                        else if ( tex.platformData->getDepth() == 8 )
                        {
                            tex.convertToPalette( rw::PALETTE_4BIT );
                        }
                        else
                        {
                            tex.convertToPalette( rw::PALETTE_8BIT );
                        }

                        tex.writeTGA(newName.c_str());

                        tex.convertToPS2();
                    }
                }

                txd.write(rwout);

                // Check how much we have actually written.
                fsOffsetNumber_t curWritePos = targetStream->TellNative();

                // If there is a difference to the original file, then notify us.
                if ( origTxdEndPos != curWritePos )
                {
                    __asm nop
                }
            }

            delete targetStream;
        }

        delete txdChat;
    }
}

struct NEWFileInterface : public rw::FileInterface
{
    filePtr_t   OpenStream( const char *streamName, const char *streamMode )
    {
        CFile *stream = fileRoot->Open( streamName, streamMode );

        return (filePtr_t)stream;
    }

    void    CloseStream( filePtr_t ptr )
    {
        CFile *stream = (CFile*)ptr;

        delete stream;
    }

    size_t  ReadStream( filePtr_t ptr, void *outBuf, size_t readCount )
    {
        CFile *stream = (CFile*)ptr;

        return stream->Read( outBuf, 1, readCount );
    }

    size_t WriteStream( filePtr_t ptr, const void *inBuf, size_t writeCount )
    {
        CFile *stream = (CFile*)ptr;

        return stream->Write( inBuf, 1, writeCount );
    }

    bool    SeekStream( filePtr_t ptr, long streamOffset, int type )
    {
        CFile *stream = (CFile*)ptr;

        return ( stream->Seek( streamOffset, type ) == 0 );
    }

    long    TellStream( filePtr_t ptr )
    {
        CFile *stream = (CFile*)ptr;

        return stream->Tell();
    }

    bool    IsEOFStream( filePtr_t ptr )
    {
        CFile *stream = (CFile*)ptr;

        return stream->IsEOF();
    }

    long    SizeStream( filePtr_t ptr )
    {
        CFile *stream = (CFile*)ptr;

        return stream->GetSize();
    }

    void    FlushStream( filePtr_t ptr )
    {
        CFile *stream = (CFile*)ptr;

        stream->Flush();
    }
};

int main( int argc, char *argv[] )
{
    // Initialize the FileSystem environment.
    fsHandle = CFileSystem::Create();

    if ( !fsHandle )
        return -1;

    // Set the global RenderWare version.
    rw::rwInterface.SetVersion( rw::SA );
    {
        // Give the correct file interface to the interface.
        NEWFileInterface fInterface;

        rw::rwInterface.SetFileInterface( &fInterface );

        // Open a handle to the GTA:SA disc and browse for the IMG files.
        CFileTranslator *discHandle = fsHandle->CreateTranslator( "E:/" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\" );

        if ( discHandle )
        {
            // Debug some obscurities.
            DebugFuncs( discHandle );

            if ( true )
            {
                // Create the build directory and get a link to it.
                bool dirCreationSuccess = fileRoot->CreateDir( "BUILD_ROOT/" );

                CFileTranslator *buildRoot = NULL;

                if ( dirCreationSuccess )
                {
                    buildRoot = fsHandle->CreateTranslator( "BUILD_ROOT/" );
                }

                if ( buildRoot )
                {
                    // Iterate through every disc file and output it into the build directory.
                    // If necessary, perform conversions.
                    _discFileTraverse traverse;
                    traverse.discHandle = discHandle;
                    traverse.buildRoot = buildRoot;

                    discHandle->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );

                    delete buildRoot;
                }
            }

            delete discHandle;
        }

        rw::rwInterface.SetFileInterface( NULL );
    }

    // Shutdown the FileSystem environment.
    CFileSystem::Destroy( fsHandle );

    return 0;
}

extern "C"
{
extern int mainCRTStartup( void );
}

extern "C" int APIENTRY _MainInit( void )
{
    DbgHeap_Init();

#if 0
    DbgTraceStackSpace stackSpace;  // reserved memory; must be always allocated.

    DbgTrace_Init( stackSpace );

    // Set up memory debugging routines.
    DbgHeap_SetMemoryAllocationWatch( _DbgHeap_MemAllocWatch );
#endif

    int ret = mainCRTStartup();

#if 0
    DbgTrace_Shutdown();
#endif
    DbgHeap_Shutdown();
    return ret;
}