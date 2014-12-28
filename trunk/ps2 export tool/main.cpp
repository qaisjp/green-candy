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
            tex.convertFromPS2();

            isPrepared = true;
        }

        // If the texture is prepared, do whatever.
        if ( isPrepared )
        {
            // Palettize the texture and convert it back into PS2 format.
            tex.convertToPalette( rw::PALETTE_8BIT );

            std::string justFileName = FileSystem::GetFileNameItem( fileNameItem, false, NULL, NULL );

            if ( false && canOutputDebug )
            {
                if (tex.platformData->getDepth() == 4)
                {
                    // Create a path to store the textures to.
                    std::string textureSaveName( justFileName );
                    textureSaveName += "_";
                    textureSaveName += tex.name;
                    textureSaveName += ".tga";

                    filePath absTexPath;

                    bool hasAbsTexPath = debugOutputRoot->GetFullPath( textureSaveName.c_str(), true, absTexPath );

                    if ( hasAbsTexPath )
                    {
                        tex.writeTGA( absTexPath.c_str() );
                    }
                }
            }

            tex.convertToPS2();

            if ( false && canOutputDebug )
            {
                if ( tex.platformData->getMipmapCount() > 1 && tex.platformData->getDepth() == 8 )
                {
                    // Save a debug image that displays the allocation scheme.
                    rw::Bitmap debugBitmap( 32, rw::RASTER_8888 );

                    // Make sure we atleast have some content.
                    debugBitmap.setSize( 10, 10 );

                    bool hasDebugBitmap = tex.platformData->getDebugBitmap( debugBitmap );

                    if ( hasDebugBitmap )
                    {
                        std::string texDebugSaveName( justFileName );
                        texDebugSaveName += "_";
                        texDebugSaveName += tex.name;
                        texDebugSaveName += "_";
                        texDebugSaveName += "debug.tga";

                        filePath absTexPath;

                        bool hasAbsTexPath = debugOutputRoot->GetFullPath( texDebugSaveName.c_str(), true, absTexPath );

                        if ( hasAbsTexPath )
                        {
                            // Create a direct3D texture and store it as TGA.
                            rw::NativeTexture newTex;
                            
                            newTex.newDirect3D();

                            // Write image data.
                            newTex.setImageData( debugBitmap );

                            // Write the TGA.
                            newTex.writeTGA( absTexPath.c_str() );
                        }
                    }
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
    CFileTranslator *debugRoot;
};

static const bool _doNormalCopy = true;

static void _discFileCallback( const filePath& discFilePathAbs, void *userdata )
{
    _discFileTraverse *info = (_discFileTraverse*)userdata;

    // Preprocess the file.
    // Create a destination file inside of the build root using the relative path from the source trans root.
    filePath relPathFromRoot;
    
    bool hasTargetRelativePath = info->discHandle->GetRelativePathFromRoot( discFilePathAbs.c_str(), true, relPathFromRoot );

    if ( hasTargetRelativePath )
    {
        bool hasPreprocessedFile = false;

        SString fileName = FileSystem::GetFileNameItem( discFilePathAbs.c_str(), true, NULL );

        SString extention;

        bool hasExtention = SharedUtil::ExtractExtention( fileName, NULL, &extention );

        if ( hasExtention )
        {
            if ( extention.CompareI( "IMG" ) == true )
            {
                printf( "processing %s ...\n", discFilePathAbs.c_str() );

                // If we have found an IMG archive, we perform the same stuff for files inside of it.
                // We copy the files into a new IMG archive tho.
                CArchiveTranslator *newIMGRoot = fileSystem->CreateIMGArchive( info->buildRoot, relPathFromRoot.c_str() );

                if ( newIMGRoot )
                {
                    CArchiveTranslator *srcIMGRoot = fileSystem->OpenIMGArchive( info->discHandle, relPathFromRoot.c_str() );

                    if ( srcIMGRoot )
                    {
                        // Run into shit.
                        _discFileTraverse traverse;
                        traverse.discHandle = srcIMGRoot;
                        traverse.buildRoot = newIMGRoot;
                        traverse.debugRoot = info->debugRoot;

                        srcIMGRoot->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );

                        printf( "writing... " );

                        newIMGRoot->Save();

                        printf( "done.\n" );

                        hasPreprocessedFile = true;
                        
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
        }

        if ( !hasPreprocessedFile )
        {
            // Do special logic for certain files.
            // Copy all files into the build root.
            CFile *sourceStream = NULL;
            {
                sourceStream = info->discHandle->Open( discFilePathAbs.c_str(), "rb" );
            }

            // Open the target stream.
            CFile *targetStream = NULL;
            {
                targetStream = info->buildRoot->Open( relPathFromRoot.c_str(), "wb" );
            }

            if ( targetStream && sourceStream )
            {
                // Allow to perform custom logic on the source and target streams.
                bool hasCopiedFile = false;
                {
                    if ( hasExtention )
                    {
                        if ( extention.CompareI( "TXD" ) == true )
                        {
                            printf( "optimizing texture container: %s ...", discFilePathAbs.c_str() );

                            bool couldProcessTXD = ProcessTXDArchive( info->discHandle, sourceStream, targetStream, info->debugRoot );

                            if ( couldProcessTXD )
                            {
                                hasCopiedFile = true;
                            }

                            printf( "OK\n" );
                        }
                    }
                }

                if ( _doNormalCopy )
                {
                    // If we have not yet created the new copy, we default to simple stream swap.
                    if ( !hasCopiedFile && targetStream )
                    {
                        // Make sure we copy from the beginning of the source stream.
                        sourceStream->Seek( 0, SEEK_SET );
                        
                        // Copy the stream contents.
                        FileSystem::StreamCopy( *sourceStream, *targetStream );
                    }
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
    }
}

inline uint8 randcolor( void )
{
    int random = rand();

    return (uint8)( ( ( (double)random) / RAND_MAX ) * 255.0f );
}

static void DebugFuncs( CFileTranslator *discHandle )
{
    // Debug a weird txd container...
    CFile *txdChat = discHandle->Open( "MODELS/GENERIC/VEHICLE.TXD", "rb" );
    //CFile *txdChat = discHandle->Open( "MODELS/GENERIC.TXD", "rb" );

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

        // Test reading our own output.
        rw::TextureDictionary txd_clone;
        {
            CFile *testOut = fileRoot->Open( "test.txd", "wb" );

            if ( testOut )
            {
                FileSystem::fileStreamBuf outbuf( testOut );
                std::ostream out_rw( &outbuf );

                if ( out_rw.good() )
                {
                    txd.write(out_rw);
                }

                delete testOut;
            }

            // Try opening the test output.
            CFile *testIn = fileRoot->Open( "test.txd", "rb" );

            if ( testIn )
            {
                FileSystem::fileStreamBuf inbuf( testIn );
                std::istream in_rw( &inbuf );

                if ( in_rw.good() )
                {
                    txd_clone.read( in_rw );
                }

                __asm nop
            }
        }

        rw::TextureDictionary& use_txd = txd_clone;

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
                if ( true )
                {
                    // Convert from PS2, convert to 4bit palette and convert back to PS2.
                    for ( unsigned int n = 0; n < use_txd.texList.size(); n++ )
                    {
                        rw::NativeTexture& tex = use_txd.texList.at( n );

                        if ( tex.name == "carpback" )
                        {
                            __asm nop
                        }

                        // WARNING: conversion does destroy some platform native data that
                        // is embedded into the texture!
                        // We need a way to retrieve the data and set it back after conversion.

                        if ( tex.platform == rw::PLATFORM_PS2 )
                        {
                            tex.convertFromPS2();

                            // Write the texture somewhere.
                            std::string newName = std::string( "txdout/" ) + tex.name;
                            newName += ".tga";

                            //tex.convertToFormat( rw::RASTER_8888 );
                            //if ( tex.platformData->getDepth() == 8 )
                            {
                                tex.convertToPalette( rw::PALETTE_8BIT );
                            }

                            tex.writeTGA(newName.c_str());

                            tex.convertToPS2();
                        }
                    }
                }

                use_txd.write(rwout);

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
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "E:/" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\" );
        CFileTranslator *discHandle = fsHandle->CreateTranslator( "D:\\gtaiso\\unpack\\gtasa\\" );

        if ( discHandle )
        {
            // Debug some obscurities.
            //DebugFuncs( discHandle );

            if ( true )
            {
                // Create the build directory and get a link to it.
                bool dirCreationSuccess = fileRoot->CreateDir( "BUILD_ROOT/" );

                CFileTranslator *buildRoot = NULL;

                if ( dirCreationSuccess )
                {
                    buildRoot = fsHandle->CreateTranslator( "BUILD_ROOT/" );
                }

                // Grab a debug root.
                CFileTranslator *debugRoot = NULL;
                {
                    filePath debugRootPath;
                    
                    bool canGetDebugRootOut = fileRoot->GetFullPath( "debugout/", false, debugRootPath );

                    if ( canGetDebugRootOut )
                    {
                        bool dirCreated = fileRoot->CreateDir( debugRootPath.c_str() );

                        if ( dirCreated )
                        {
                            debugRoot = fileSystem->CreateTranslator( debugRootPath.c_str() );
                        }
                    }
                }

                if ( buildRoot )
                {
                    // Iterate through every disc file and output it into the build directory.
                    // If necessary, perform conversions.
                    _discFileTraverse traverse;
                    traverse.discHandle = discHandle;
                    traverse.buildRoot = buildRoot;
                    traverse.debugRoot = debugRoot;

                    discHandle->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );

                    delete buildRoot;
                }

                if ( debugRoot )
                {
                    delete debugRoot;
                }

                rw::NativeTexture::DebugParameters();
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