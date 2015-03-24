#include "StdInc.h"

#include <renderware.h>
#include <fstream>

#include <SharedUtil.hpp>
#include <iostream>
#include <streambuf>

static CFileSystem *fsHandle = NULL;

static bool _outputOptimizedTGA = false;

static rw::Interface *engineInterface = NULL;

#if 0

static bool ProcessTXDArchive( CFileTranslator *srcRoot, CFile *srcStream, CFile *targetStream, CFileTranslator *debugOutputRoot )
{
    bool hasProcessed = false;

    // Optimize the texture archive.
    FileSystem::fileStreamBuf buf( srcStream );
    std::istream rw( &buf );

    rw::TextureDictionary txd;
    
    if ( rw.good() )
    {
        try
        {
            txd.read(rw);
        }
        catch( rw::RwException& )
        {
            return false;
        }
    }

    const filePath& texPath = srcStream->GetPath();

    bool canOutputDebug = false;
    bool canOutputDebugSpecial = false;

    filePath fileNameItem;

    bool hasDebugName = srcRoot->GetRelativePathFromRoot( texPath.c_str(), true, fileNameItem );

    if ( hasDebugName && debugOutputRoot )
    {
        // Create a directory to save textures in (in TGA format).
        {
            std::string dirName( "conv_tga/" );

            bool createDirectorySuccess = debugOutputRoot->CreateDir( dirName.c_str() );

            if ( createDirectorySuccess )
            {
                canOutputDebug = true;
            }
        }

        {
            std::string specialDirName = "special_txd/";

            bool createDirectorySuccess = debugOutputRoot->CreateDir( specialDirName.c_str() );

            if ( createDirectorySuccess )
            {
                canOutputDebugSpecial = true;
            }
        }
    }

    // Process all textures.
    bool processSuccessful = true;

    bool isSpecialTXD = false;

    try
    {
        for ( unsigned int n = 0; n < txd.texList.size(); n++ )
        {
            rw::NativeTexture& tex = txd.texList[n];

            // Make sure are in Direct3D 9 format.
            bool isPrepared = tex.convertToDirect3D9();

            // If the texture is prepared, do whatever.
            if ( isPrepared )
            {
#if 0
                // Palettize the texture and convert it back into PS2 format.
                if (tex.platformData->getPaletteType() == rw::PALETTE_8BIT)
                {
                    tex.convertToPalette( rw::PALETTE_4BIT );
                }
                else
                {
                    tex.convertToPalette( rw::PALETTE_8BIT );
                }
#endif

                std::string justFileName = FileSystem::GetFileNameItem( fileNameItem, false, NULL, NULL );

                if ( canOutputDebug )
                {
                    //if ( /*( tex.platformData->getWidth() != tex.platformData->getHeight() || tex.platformData->getDepth() != 8 ) &&*/ tex.platformData->isCompressed() == false )
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
                            tex.writeTGA( absTexPath.c_str(), _outputOptimizedTGA );
                        }

                        isSpecialTXD = true;
                    }
                }

                tex.convertToPS2();

                if ( false && canOutputDebug )
                {
                    if ( tex.platformData->getMipmapCount() > 1 )
                    {
                        // Save a debug image that displays the allocation scheme.
                        rw::Bitmap debugBitmap( 32, rw::RASTER_8888, rw::COLOR_RGBA );

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
                                newTex.writeTGA( absTexPath.c_str(), false );
                            }
                        }
                    }
                }
            }
        }
    }
    catch( rw::RwException& )
    {
        processSuccessful = false;
    }

    // We do not perform any error checking for now.
    if ( processSuccessful )
    {
        // Write the TXD into the target stream.
        FileSystem::fileStreamBuf writebuf( targetStream );
        std::ostream rwout( &writebuf );

        if ( rwout.good() )
        {
            try
            {
                txd.write(rwout);

                hasProcessed = true;
            }
            catch( rw::RwException& )
            {
                hasProcessed = false;
            }
        }
    }

    // Also write a raw version to the debug folder if we are a special TXD.
    if ( isSpecialTXD && canOutputDebugSpecial )
    {
        filePath outPath = filePath( "special_txd/" ) + fileNameItem;

        CFile *outFile = debugOutputRoot->Open( outPath.c_str(), "wb" );

        if ( outFile )
        {
            srcStream->Seek( 0, SEEK_SET );

            FileSystem::StreamCopy( *srcStream, *outFile );

            outFile->SetSeekEnd();

            delete outFile;
        }
    }

    return hasProcessed;
}

static const bool _doNormalCopy = false;

struct _discFileSentry
{
    CFileTranslator *debugRoot;

    inline bool OnSingletonFile(
        CFileTranslator *sourceRoot, CFileTranslator *buildRoot, const filePath& relPathFromRoot,
        const SString& fileName, const SString& extention, CFile *sourceStream,
        bool isInArchive
    )
    {
        // Open the target stream.
        CFile *targetStream = NULL;
        {
            targetStream = buildRoot->Open( relPathFromRoot.c_str(), "wb" );
        }

        if ( targetStream && sourceStream )
        {
            // Allow to perform custom logic on the source and target streams.
            bool hasCopiedFile = false;
            {
                if ( extention.CompareI( "TXD" ) == true )
                {
                    printf( "optimizing texture container: %s ...", relPathFromRoot.c_str() );

                    bool couldProcessTXD = ProcessTXDArchive( sourceRoot, sourceStream, targetStream, this->debugRoot );

                    if ( couldProcessTXD )
                    {
                        hasCopiedFile = true;

                        printf( "OK\n" );
                    }
                    else
                    {
                        printf( "error\n" );
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

        return true;
    }

    inline void OnArchiveFail( const SString& fileName, const SString& extention )
    {
        printf( "failed to create new IMG archive for processing; defaulting to file-copy ...\n" );
    }
};

#endif

static void DebugFuncs( CFileTranslator *discHandle )
{
    // Debug deserialisation.
    rw::streamConstructionFileParam_t fileParam( "txdgen_in/hq_roads/backroad_sfs.txd" );

    rw::Stream *rwStream = engineInterface->CreateStream( rw::RWSTREAMTYPE_FILE, rw::RWSTREAMMODE_READONLY, &fileParam );

    if ( rwStream != NULL )
    {
        // Deserialize.
        rw::RwObject *newObject = engineInterface->Deserialize( rwStream );

        if ( newObject )
        {
            rw::TexDictionary *texDict = rw::ToTexDictionary( engineInterface, newObject );

            // Do some stuff.
            if ( texDict )
            {
                for ( rw::TexDictionary::texIter_t iter = texDict->GetTextureIterator(); !iter.IsEnd(); iter.Increment() )
                {
                    rw::TextureBase *texture = iter.Resolve();

                    // Convert the texture to ATC.
                    rw::Raster *texRaster = texture->GetRaster();

                    if ( texRaster )
                    {
                        bool conversionSuccess = rw::ConvertRasterTo( texRaster, "ATI_Compress" );

                        __asm nop
                    }
                }
            }

            __asm int 3
        }

        engineInterface->DeleteStream( rwStream );
    }

#if 0
    // Debug a weird txd container...
    //CFile *txdChat = discHandle->Open( "MODELS/GENERIC/VEHICLE.TXD", "rb" );
    //CFile *txdChat = discHandle->Open( "MODELS/GENERIC.TXD", "rb" );
    CFile *txdChat = discHandle->Open( "generic_PVR.txd", "rb" );

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

#if 0
                        if ( tex.name == "ketchup" )
                        {
                            __asm nop
                        }
#endif

                        // WARNING: conversion does destroy some platform native data that
                        // is embedded into the texture!
                        // We need a way to retrieve the data and set it back after conversion.

                        uint32 origPlatform = tex.platform;

                        if ( origPlatform == rw::PLATFORM_PS2 )
                        {
                            tex.convertFromPS2();
                        }
                        else if ( origPlatform == rw::PLATFORM_XBOX )
                        {
                            tex.convertFromXbox();
                        }
                        else if ( origPlatform == rw::PLATFORM_PVR )
                        {
                            tex.convertFromPVR();
                        }

                        tex.convertToPVR();
                        tex.convertFromPVR();

                        // Write the texture somewhere.
                        std::string newName = std::string( "txdout/" ) + tex.name;
                        newName += ".tga";

                        //tex.convertToFormat( rw::RASTER_8888 );
                        //if ( tex.platformData->getDepth() == 8 )
                        {
                            //tex.convertToPalette( rw::PALETTE_8BIT );
                        }

                        tex.writeTGA(newName.c_str());

                        if ( origPlatform == rw::PLATFORM_PS2 )
                        {
                            tex.convertToPS2();
                        }
                        else if ( origPlatform == rw::PLATFORM_XBOX )
                        {
                            tex.convertToXbox();
                        }
                        else if ( origPlatform == rw::PLATFORM_PVR )
                        {
                            tex.convertToPVR();
                        }
                        // else we just keep it Direct3D/PC format.
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
#endif
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

    // Create a RenderWare engine.
    rw::LibraryVersion wantedVersion = rw::KnownVersions::getGameVersion( rw::KnownVersions::SA );

    engineInterface = rw::CreateEngine( wantedVersion );

    if ( engineInterface != NULL )
    {
        // Give the correct file interface to the interface.
        NEWFileInterface fInterface;

        engineInterface->SetFileInterface( &fInterface );

        engineInterface->SetPaletteRuntime( rw::PALRUNTIME_PNGQUANT );

        engineInterface->SetDXTRuntime( rw::DXTRUNTIME_SQUISH );

        // Open a handle to the GTA:SA disc and browse for the IMG files.
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "E:/" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "C:\\Program Files (x86)\\Rockstar Games\\GTA San Andreas\\" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "C:\\Program Files (x86)\\Rockstar Games\\GTAIII\\" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "D:\\gtaiso\\unpack\\gta3_xbox\\" );
        //CFileTranslator *discHandle = fsHandle->CreateTranslator( "txdgen_in/xbox_swizzle_samples/" );
        CFileTranslator *discHandle = fsHandle->CreateTranslator( "debugtxd/" );

        if ( discHandle )
        {
            // Debug some obscurities.
            DebugFuncs( discHandle );

            if ( false )
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
#if 0
                    // Iterate through every disc file and output it into the build directory.
                    // If necessary, perform conversions.
                    gtaFileProcessor <_discFileSentry> fileProc;

                    // Change this if required!
                    fileProc.setUseCompressedIMGArchives( true );
                    fileProc.setArchiveReconstruction( false );

                    _discFileSentry sentry;
                    sentry.debugRoot = debugRoot;

                    fileProc.process( &sentry, discHandle, buildRoot );

                    delete buildRoot;
#endif
                }

                if ( debugRoot )
                {
                    delete debugRoot;
                }
            }

            delete discHandle;
        }

        // Clear engine links.
        engineInterface->SetFileInterface( NULL );

        // Destroy the RenderWare engine again.
        rw::DeleteEngine( engineInterface );
    }

    // Shutdown the FileSystem environment.
    CFileSystem::Destroy( fsHandle );

    return 0;
}