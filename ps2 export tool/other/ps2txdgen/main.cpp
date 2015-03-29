#include "StdInc.h"

#include <renderware.h>
#include <fstream>

#include <SharedUtil.hpp>
#include <iostream>
#include <streambuf>

#include <signal.h>

rw::Interface *rwEngine = NULL;

static CFileSystem *fsHandle = NULL;

static bool _please_terminate = false;

struct termination_request
{
};

enum eTargetPlatform
{
    PLATFORM_PC,
    PLATFORM_PS2,
    PLATFORM_XBOX,
    PLATFORM_DXT_MOBILE,
    PLATFORM_PVR,
    PLATFORM_ATC,
    PLATFORM_UNC_MOBILE
};


struct RwWarningBuffer : public rw::WarningManagerInterface
{
    std::string buffer;

    void Purge( void )
    {
        // Output the content to the stream.
        if ( !buffer.empty() )
        {
            std::cout <<
                "- Warnings:\n";

            std::cout << buffer << std::endl;

            buffer.clear();
        }
    }

    virtual void OnWarning( const std::string& message )
    {
        if ( !buffer.empty() )
        {
            buffer += '\n';
        }

        buffer += message;
    }
};

static RwWarningBuffer _warningMan;

inline bool _meetsDebugCriteria( const rw::TextureBase& tex )
{
    return true;//( tex.getMipmapCount() > 1 );
}

inline rw::Stream* RwStreamCreateTranslated( CFile *eirStream )
{
    rw::streamConstructionCustomParam_t customParam( "eirfs_file", eirStream );

    rw::Stream *result = rwEngine->CreateStream( rw::RWSTREAMTYPE_CUSTOM, rw::RWSTREAMMODE_READWRITE, &customParam );

    return result;
}

static bool ProcessTXDArchive(
    CFileTranslator *srcRoot, CFile *srcStream, CFile *targetStream, eTargetPlatform targetPlatform,
    bool clearMipmaps,
    bool generateMipmaps, rw::eMipmapGenerationMode mipGenMode, uint32 mipGenMaxLevel,
    bool improveFiltering,
    bool doCompress, float compressionQuality,
    bool outputDebug, CFileTranslator *debugRoot,
    rw::KnownVersions::eGameVersion gameVersion,
    std::string& errMsg
)
{
    bool hasProcessed = false;

    // Optimize the texture archive.
    rw::Stream *txd_stream = RwStreamCreateTranslated( srcStream );

    try
    {
        rw::TexDictionary *txd = NULL;
        
        if ( txd_stream != NULL )
        {
            try
            {
                rw::RwObject *rwObj = rwEngine->Deserialize( txd_stream );

                if ( rwObj )
                {
                    txd = rw::ToTexDictionary( rwEngine, rwObj );

                    if ( txd == NULL )
                    {
                        errMsg = "not a texture dictionary (";
                        errMsg += rwEngine->GetObjectTypeName( rwObj );
                        errMsg += ")";

                        rwEngine->DeleteRwObject( rwObj );
                    }
                }
                else
                {
                    errMsg = "unknown RenderWare stream (maybe compressed)";
                }
            }
            catch( rw::RwException& except )
            {
                errMsg = "error reading txd: " + except.message;
                
                throw;
            }
        }

        if ( txd )
        {
            // Update the version of this texture dictionary.
            txd->SetEngineVersion( rwEngine->GetVersion() );

            try
            {
                // Process all textures.
                bool processSuccessful = true;

                try
                {
                    for ( rw::TexDictionary::texIter_t iter = txd->GetTextureIterator(); !iter.IsEnd(); iter.Increment() )
                    {
                        rw::TextureBase *theTexture = iter.Resolve();

                        // Update the version of this texture.
                        theTexture->SetEngineVersion( rwEngine->GetVersion() );

                        // We need to modify the raster.
                        rw::Raster *texRaster = theTexture->GetRaster();

                        if ( texRaster )
                        {
#if 0
                            // Clear mipmaps if requested.
                            if ( clearMipmaps )
                            {
                                tex.clearMipmaps();
                            }

                            // Generate mipmaps on demand.
                            if ( generateMipmaps )
                            {
                                // We generate as many mipmaps as we can.
                                tex.generateMipmaps( mipGenMaxLevel + 1, mipGenMode );
                            }
#endif

                            // Output debug stuff.
                            if ( outputDebug && debugRoot != NULL )
                            {
                                // We want to debug mipmap generation, so output debug textures only using mipmaps.
                                //if ( _meetsDebugCriteria( tex ) )
                                {
                                    const filePath& srcPath = srcStream->GetPath();

                                    filePath relSrcPath;

                                    bool hasRelSrcPath = srcRoot->GetRelativePathFromRoot( srcPath.c_str(), true, relSrcPath );

                                    if ( hasRelSrcPath )
                                    {
                                        // Create a unique filename for this texture.
                                        std::string directoryPart;

                                        std::string fileNamePart = FileSystem::GetFileNameItem( relSrcPath.c_str(), false, &directoryPart, NULL );

                                        if ( fileNamePart.size() != 0 )
                                        {
                                            std::string uniqueTextureNameTGA = directoryPart + fileNamePart + "_" + theTexture->name + ".tga";

                                            CFile *debugOutputStream = debugRoot->Open( uniqueTextureNameTGA.c_str(), "wb" );

                                            if ( debugOutputStream )
                                            {
                                                // Create a debug raster.
                                                rw::Raster *newRaster = rw::CreateRaster( rwEngine );

                                                if ( newRaster )
                                                {
                                                    newRaster->newNativeData( "Direct3D" );

                                                    // Put the debug content into it.
                                                    {
                                                        rw::Bitmap debugTexContent;

                                                        debugTexContent.setBgColor( 1, 1, 1 );

                                                        bool gotDebugContent = rw::DebugDrawMipmaps( rwEngine, texRaster, debugTexContent );

                                                        if ( gotDebugContent )
                                                        {
                                                            newRaster->setImageData( debugTexContent );
                                                        }
                                                    }

                                                    if ( newRaster->getMipmapCount() > 0 )
                                                    {
                                                        // Write the debug texture to it.
                                                        rw::Stream *outputStream = RwStreamCreateTranslated( debugOutputStream );

                                                        if ( outputStream )
                                                        {
                                                            newRaster->writeTGAStream( outputStream, false );

                                                            rwEngine->DeleteStream( outputStream );
                                                        }
                                                    }

                                                    rw::DeleteRaster( newRaster );
                                                }

                                                // Free the stream handle.
                                                delete debugOutputStream;
                                            }
                                        }
                                    }
                                }
                            }

#if 0
                            // Palettize the texture to save space.
                            if ( doCompress )
                            {
                                if ( targetPlatform == PLATFORM_PS2 )
                                {
                                    tex.optimizeForLowEnd( compressionQuality );
                                }
                                else if ( targetPlatform == PLATFORM_XBOX || targetPlatform == PLATFORM_PC )
                                {
                                    // Compress if we are not already compressed.
                                    if ( tex.platformData->isCompressed() == false )
                                    {
                                        tex.platformData->compress( compressionQuality );
                                    }
                                }
                            }
#endif

                            // Improve the filtering mode if the user wants us to.
                            if ( improveFiltering )
                            {
                                tex.improveFiltering();
                            }

                            // Convert it into the target platform.
                            if ( targetPlatform == PLATFORM_PS2 )
                            {
                                rw::ConvertRasterTo( texRaster, "PlayStation2" );
                            }
                            else if ( targetPlatform == PLATFORM_XBOX )
                            {
                                rw::ConvertRasterTo( texRaster, "XBOX" );
                            }
                            else if ( targetPlatform == PLATFORM_PC )
                            {
                                // First, convert to Direct3D driver.
                                rw::ConvertRasterTo( texRaster, "Direct3D" );

#if 0
                                // Next, depends on the game.
                                if (gameVersion == rw::KnownVersions::SA)
                                {
                                    rw::ConvertRasterTo( texRaster, "Direct3D 9" );
                                }
                                else
                                {
                                    rw::ConvertRasterTo( texRaster, "Direct3D 8" );
                                }

                                // Make sure that we have a D3DFORMAT field.
                                tex.makeDirect3DCompatible();
#endif
                            }
                            else if ( targetPlatform == PLATFORM_DXT_MOBILE )
                            {
                                rw::ConvertRasterTo( texRaster, "s3tc_mobile" );
                            }
                            else if ( targetPlatform == PLATFORM_PVR )
                            {
                                rw::ConvertRasterTo( texRaster, "PowerVR" );
                            }
                            else if ( targetPlatform == PLATFORM_ATC )
                            {
                                rw::ConvertRasterTo( texRaster, "ATI_Compress" );
                            }
                            else if ( targetPlatform == PLATFORM_UNC_MOBILE )
                            {
                                rw::ConvertRasterTo( texRaster, "uncompressed_mobile" );
                            }
                            else
                            {
                                assert( 0 );
                            }
                        }
                    }
                }
                catch( rw::RwException& except )
                {
                    errMsg = "error processing textures: " + except.message;
                    
                    throw;
                }

#if 0
                // Introduce this stub when dealing with memory leaks.
                static int ok_num = 0;

                if ( ok_num++ > 100 )
                {
                    throw termination_request();
                }
#endif

                // We do not perform any error checking for now.
                if ( processSuccessful )
                {
                    // Write the TXD into the target stream.
                    rw::Stream *rwTargetStream = RwStreamCreateTranslated( targetStream );

                    if ( targetStream )
                    {
                        try
                        {
                            try
                            {
                                rwEngine->Serialize( txd, rwTargetStream );
                            }
                            catch( rw::RwException& except )
                            {
                                errMsg = "error writing txd: " + except.message;
                                
                                throw;
                            }
                        }
                        catch( ... )
                        {
                            rwEngine->DeleteStream( rwTargetStream );

                            throw;
                        }

                        rwEngine->DeleteStream( rwTargetStream );

                        hasProcessed = true;
                    }
                }
            }
            catch( ... )
            {
                rwEngine->DeleteRwObject( txd );

                txd = NULL;
                
                throw;
            }

            // Delete the TXD.
            rwEngine->DeleteRwObject( txd );
        }
    }
    catch( rw::RwException& )
    {
        hasProcessed = false;
    }
    catch( ... )
    {
        hasProcessed = false;

        if ( txd_stream )
        {
            rwEngine->DeleteStream( txd_stream );

            txd_stream = NULL;
        }

        throw;
    }

    if ( txd_stream )
    {
        rwEngine->DeleteStream( txd_stream );

        txd_stream = NULL;
    }

    return hasProcessed;
}

struct _discFileSentry
{
    eTargetPlatform targetPlatform;
    bool clearMipmaps;
    bool generateMipmaps;
    rw::eMipmapGenerationMode mipGenMode;
    uint32 mipGenMaxLevel;
    bool improveFiltering;
    bool doCompress;
    float compressionQuality;
    rw::KnownVersions::eGameVersion gameVersion;
    bool outputDebug;
    CFileTranslator *debugTranslator;

    inline bool OnSingletonFile(
        CFileTranslator *sourceRoot, CFileTranslator *buildRoot, const filePath& relPathFromRoot,
        const SString& fileName, const SString& extention, CFile *sourceStream,
        bool isInArchive
    )
    {
        // If we are asked to terminate, just do it.
        if ( _please_terminate )
        {
            throw termination_request();
        }

        // Decide whether we need a copy.
        bool requiresCopy = false;

        bool anyWork = false;

        if ( extention.CompareI( "TXD" ) == true || isInArchive )
        {
            requiresCopy = true;
        }

        // Open the target stream.
        CFile *targetStream = NULL;

        if ( requiresCopy )
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
                    printf( "*** %s ...", relPathFromRoot.c_str() );

                    std::string errorMessage;

                    bool couldProcessTXD = ProcessTXDArchive(
                        sourceRoot, sourceStream, targetStream, this->targetPlatform,
                        this->clearMipmaps,
                        this->generateMipmaps, this->mipGenMode, this->mipGenMaxLevel,
                        this->improveFiltering,
                        this->doCompress, this->compressionQuality,
                        this->outputDebug, this->debugTranslator,
                        this->gameVersion,
                        errorMessage
                    );

                    if ( couldProcessTXD )
                    {
                        hasCopiedFile = true;

                        anyWork = true;

                        printf( "OK\n" );
                    }
                    else
                    {
                        printf( "error: " );

                        std::cout << std::endl << errorMessage << std::endl;
                    }

                    // Output any warnings.
                    _warningMan.Purge();
                }
            }

            if ( requiresCopy )
            {
                // If we have not yet created the new copy, we default to simple stream swap.
                if ( !hasCopiedFile && targetStream )
                {
                    // Make sure we copy from the beginning of the source stream.
                    sourceStream->Seek( 0, SEEK_SET );
                    
                    // Copy the stream contents.
                    FileSystem::StreamCopy( *sourceStream, *targetStream );

                    hasCopiedFile = true;
                }
            }
        }

        if ( targetStream )
        {
            delete targetStream;
        }

        return anyWork;
    }

    inline void OnArchiveFail( const SString& fileName, const SString& extention )
    {
        printf( "failed to create new IMG archive for processing; defaulting to file-copy ...\n" );
    }
};

static bool _has_terminated = false;

void _term_handler( int msg )
{
    // Please terminate :(
    _please_terminate = true;

    while ( !_has_terminated )
    {
        Sleep( 10000 );
    }
}

struct TxdGenExceptionHandler : public DbgTrace::IExceptionHandler
{
    virtual bool OnException( unsigned int error_code, DbgTrace::IEnvSnapshot *snapshot )
    {
        printf(
            "\n\n\nTxdGen has crashed. Please report back to The_GTA so he can fix this issue.\n\n"
        );

        if ( snapshot )
        {
            printf( "callstack:\n" );

            // Print out debug info for the user.
            std::string callstackString = snapshot->ToString();

            printf( "%s", callstackString.c_str() );
        }
        else
        {
            // There sadly is no debug information.
            printf(
                "no debug info available.\n"
            );
        }

        printf( "\npress enter to get rekt..." );

#ifndef _DEBUG
        // Wait for the user to recognize this.
        getchar();

        // Terminate shit.
        TerminateProcess( GetCurrentProcess(), -2 );
#endif

        // We want to get to the debugger.
        return false;
    }
};

struct eirFileSystemMetaInfo
{
    inline eirFileSystemMetaInfo( void )
    {
        this->theStream = NULL;
    }

    inline ~eirFileSystemMetaInfo( void )
    {
        return;
    }

    CFile *theStream;
};

struct eirFileSystemWrapperProvider : public rw::customStreamInterface
{
    void OnConstruct( rw::eStreamMode streamMode, void *userdata, void *membuf, size_t memSize ) const
    {
        eirFileSystemMetaInfo *meta = new (membuf) eirFileSystemMetaInfo;

        meta->theStream = (CFile*)userdata;
    }

    void OnDestruct( void *memBuf, size_t memSize ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        meta->~eirFileSystemMetaInfo();
    }

    size_t Read( void *memBuf, void *out_buf, size_t readCount ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        return meta->theStream->Read( out_buf, 1, readCount );
    }

    size_t Write( void *memBuf, const void *in_buf, size_t writeCount ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        return meta->theStream->Write( in_buf, 1, writeCount );
    }

    void Skip( void *memBuf, size_t skipCount ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        meta->theStream->SeekNative( skipCount, SEEK_CUR );
    }

    rw::int64 Tell( const void *memBuf ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        return meta->theStream->TellNative();
    }

    void Seek( void *memBuf, rw::int64 stream_offset, rw::eSeekMode seek_mode ) const
    {
        int ansi_seek = SEEK_SET;

        if ( seek_mode == rw::RWSEEK_BEG )
        {
            ansi_seek = SEEK_SET;
        }
        else if ( seek_mode == rw::RWSEEK_CUR )
        {
            ansi_seek = SEEK_CUR;
        }
        else if ( seek_mode == rw::RWSEEK_END )
        {
            ansi_seek = SEEK_END;
        }
        else
        {
            assert( 0 );
        }

        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        meta->theStream->SeekNative( stream_offset, ansi_seek );
    }

    rw::int64 Size( const void *memBuf ) const
    {
        eirFileSystemMetaInfo *meta = (eirFileSystemMetaInfo*)memBuf;

        return meta->theStream->GetSizeNative();
    }

    bool SupportsSize( const void *memBuf ) const
    {
        return true;
    }

    CFileSystem *nativeFileSystem;
};

inline bool isGoodEngine( const rw::Interface *engineInterface )
{
    if ( engineInterface->IsObjectRegistered( "texture" ) == false )
        return false;

    if ( engineInterface->IsObjectRegistered( "texture_dictionary" ) == false )
        return false;

    // We are ready to go.
    return true;
}

bool ApplicationMain( void )
{
    signal( SIGTERM, _term_handler );
    signal( SIGBREAK, _term_handler );

    std::cout <<
        "RenderWare TXD generator tool by The_GTA. Compiled on " __DATE__ "\n" \
        "Use this tool at your own risk!\n\n" \
        "Visit mtasa.com\n\n";

    bool successful = true;

    // Create a RenderWare engine.
    rw::LibraryVersion defaultLibVer;

    defaultLibVer.rwLibMajor = 3;
    defaultLibVer.rwLibMinor = 0;
    defaultLibVer.rwRevMajor = 0;
    defaultLibVer.rwRevMinor = 0;
    defaultLibVer.buildNumber = 0xFFFF;

    rwEngine = rw::CreateEngine( defaultLibVer );

    if ( rwEngine != NULL )
    {
        if ( isGoodEngine( rwEngine ) )
        {
            // Initialize environments.
            fsHandle = CFileSystem::Create();

            // Register the native file system wrapper type.
            eirFileSystemWrapperProvider eirfs_file_wrap;
            eirfs_file_wrap.nativeFileSystem = fsHandle;

            rwEngine->RegisterStream( "eirfs_file", sizeof( eirFileSystemMetaInfo ), &eirfs_file_wrap );
            
            // By default, we create San Andreas files.
            rw::KnownVersions::eGameVersion c_gameVersion = rw::KnownVersions::SA;

            // Set up the warning buffer.
            rwEngine->SetWarningManager( &_warningMan );

            // Read the configuration file.
            CINI *configFile = LoadINI( "txdgen.ini" );

            std::string c_outputRoot = "txdgen_out/";
            std::string c_gameRoot = "txdgen/";

            eTargetPlatform c_targetPlatform = PLATFORM_PC;

            bool c_clearMipmaps = false;

            bool c_generateMipmaps = false;

            rw::eMipmapGenerationMode c_mipGenMode = rw::MIPMAPGEN_DEFAULT;

            uint32 c_mipGenMaxLevel = 0;

            bool c_improveFiltering = true;

            bool compressTextures = false;

            rw::ePaletteRuntimeType c_palRuntimeType = rw::PALRUNTIME_NATIVE;

            rw::eDXTCompressionMethod c_dxtRuntimeType = rw::DXTRUNTIME_NATIVE;

            bool c_reconstructIMGArchives = true;

            bool c_fixIncompatibleRasters = true;
            bool c_dxtPackedDecompression = false;

            bool c_imgArchivesCompressed = false;

            bool c_ignoreSerializationRegions = false;

            float c_compressionQuality = 0.5f;

            bool c_outputDebug = false;

            if ( configFile )
            {
                if ( CINI::Entry *mainEntry = configFile->GetEntry( "Main" ) )
                {
                    // Output root.
                    if ( const char *newOutputRoot = mainEntry->Get( "outputRoot" ) )
                    {
                        c_outputRoot = newOutputRoot;
                    }

                    // Game root.
                    if ( const char *newGameRoot = mainEntry->Get( "gameRoot" ) )
                    {
                        c_gameRoot = newGameRoot;
                    }

                    // Target Platform.
                    const char *targetPlatform = mainEntry->Get( "targetPlatform" );

                    if ( targetPlatform )
                    {
                        if ( stricmp( targetPlatform, "PC" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_PC;
                        }
                        else if ( stricmp( targetPlatform, "PS2" ) == 0 ||
                                  stricmp( targetPlatform, "Playstation 2" ) == 0 ||
                                  stricmp( targetPlatform, "PlayStation2" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_PS2;
                        }
                        else if ( stricmp( targetPlatform, "XBOX" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_XBOX;
                        }
                        else if ( stricmp( targetPlatform, "DXT_MOBILE" ) == 0 ||
                                  stricmp( targetPlatform, "S3TC_MOBILE" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_DXT_MOBILE;
                        }
                        else if ( stricmp( targetPlatform, "PVR" ) == 0 ||
                                  stricmp( targetPlatform, "PowerVR" ) == 0 ||
                                  stricmp( targetPlatform, "PVRTC" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_PVR;
                        }
                        else if ( stricmp( targetPlatform, "ATC" ) == 0 ||
                                  stricmp( targetPlatform, "ATI_Compress" ) == 0 ||
                                  stricmp( targetPlatform, "ATI" ) == 0 ||
                                  stricmp( targetPlatform, "ATITC" ) == 0 ||
                                  stricmp( targetPlatform, "ATI TC" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_ATC;
                        }
                        else if ( stricmp( targetPlatform, "UNC" ) == 0 ||
                                  stricmp( targetPlatform, "UNCOMPRESSED" ) == 0 ||
                                  stricmp( targetPlatform, "unc_mobile" ) == 0 ||
                                  stricmp( targetPlatform, "uncompressed_mobile" ) == 0 )
                        {
                            c_targetPlatform = PLATFORM_UNC_MOBILE;
                        }
                    }

                    // Target game version.
                    if ( const char *targetVersion = mainEntry->Get( "targetVersion" ) )
                    {
                        rw::KnownVersions::eGameVersion gameVer;
                        bool hasGameVer = false;
                            
                        if ( stricmp( targetVersion, "SA" ) == 0 ||
                             stricmp( targetVersion, "SanAndreas" ) == 0 ||
                             stricmp( targetVersion, "San Andreas" ) == 0 ||
                             stricmp( targetVersion, "GTA SA" ) == 0 ||
                             stricmp( targetVersion, "GTASA" ) == 0 )
                        {
                            gameVer = rw::KnownVersions::SA;

                            hasGameVer = true;
                        }
                        else if ( stricmp( targetVersion, "VC" ) == 0 ||
                                  stricmp( targetVersion, "ViceCity" ) == 0 ||
                                  stricmp( targetVersion, "Vice City" ) == 0 ||
                                  stricmp( targetVersion, "GTA VC" ) == 0 ||
                                  stricmp( targetVersion, "GTAVC" ) == 0 )
                        {
                            if ( c_targetPlatform == PLATFORM_PS2 )
                            {
                                gameVer = rw::KnownVersions::VC_PS2;
                            }
                            else
                            {
                                gameVer = rw::KnownVersions::VC_PC;
                            }

                            hasGameVer = true;
                        }
                        else if ( stricmp( targetVersion, "GTAIII" ) == 0 ||
                                  stricmp( targetVersion, "III" ) == 0 ||
                                  stricmp( targetVersion, "GTA3" ) == 0 ||
                                  stricmp( targetVersion, "GTA 3" ) == 0 )
                        {
                            gameVer = rw::KnownVersions::GTA3;

                            hasGameVer = true;
                        }
                        else if ( stricmp( targetVersion, "MANHUNT" ) == 0 ||
                                  stricmp( targetVersion, "MHUNT" ) == 0 ||
                                  stricmp( targetVersion, "MH" ) == 0 )
                        {
                            gameVer = rw::KnownVersions::MANHUNT;

                            hasGameVer = true;
                        }
                        
                        if ( hasGameVer )
                        {
                            c_gameVersion = gameVer;
                        }
                    }

                    // Mipmap clear flag.
                    if ( mainEntry->Find( "clearMipmaps" ) )
                    {
                        c_clearMipmaps = mainEntry->GetBool( "clearMipmaps" );
                    }

                    // Mipmap Generation enable.
                    if ( mainEntry->Find( "generateMipmaps" ) )
                    {
                        c_generateMipmaps = mainEntry->GetBool( "generateMipmaps" );
                    }

                    // Mipmap Generation Mode.
                    if ( const char *mipGenMode = mainEntry->Get( "mipGenMode" ) )
                    {
                        if ( stricmp( mipGenMode, "default" ) == 0 )
                        {
                            c_mipGenMode = rw::MIPMAPGEN_DEFAULT;
                        }
                        else if ( stricmp( mipGenMode, "contrast" ) == 0 )
                        {
                            c_mipGenMode = rw::MIPMAPGEN_CONTRAST;
                        }
                        else if ( stricmp( mipGenMode, "brighten" ) == 0 )
                        {
                            c_mipGenMode = rw::MIPMAPGEN_BRIGHTEN;
                        }
                        else if ( stricmp( mipGenMode, "darken" ) == 0 )
                        {
                            c_mipGenMode = rw::MIPMAPGEN_DARKEN;
                        }
                        else if ( stricmp( mipGenMode, "selectclose" ) == 0 )
                        {
                            c_mipGenMode = rw::MIPMAPGEN_SELECTCLOSE;
                        } 
                    }

                    // Mipmap generation maximum level.
                    if ( mainEntry->Find( "mipGenMaxLevel" ) )
                    {
                        int mipGenMaxLevelInt = mainEntry->GetInt( "mipGenMaxLevel" );

                        if ( mipGenMaxLevelInt >= 0 )
                        {
                            c_mipGenMaxLevel = (uint32)mipGenMaxLevelInt;
                        }
                    }

                    // Filter mode improvement.
                    if ( mainEntry->Find( "improveFiltering" ) )
                    {
                        c_improveFiltering = mainEntry->GetBool( "improveFiltering" );
                    }

                    // Compression.
                    if ( mainEntry->Find( "compressTextures" ) )
                    {
                        compressTextures = mainEntry->GetBool( "compressTextures" );
                    }

                    // Compression quality.
                    if ( mainEntry->Find( "compressionQuality" ) )
                    {
                        c_compressionQuality = (float)mainEntry->GetFloat( "compressionQuality", 0.0 );
                    }

                    // Palette runtime type.
                    if ( const char *palRuntimeType = mainEntry->Get( "palRuntimeType" ) )
                    {
                        if ( stricmp( palRuntimeType, "native" ) == 0 )
                        {
                            c_palRuntimeType = rw::PALRUNTIME_NATIVE;
                        }
                        else if ( stricmp( palRuntimeType, "pngquant" ) == 0 )
                        {
                            c_palRuntimeType = rw::PALRUNTIME_PNGQUANT;
                        }
                    }

                    // DXT compression method.
                    if ( const char *dxtCompressionMethod = mainEntry->Get( "dxtRuntimeType" ) )
                    {
                        if ( stricmp( dxtCompressionMethod, "native" ) == 0 )
                        {
                            c_dxtRuntimeType = rw::DXTRUNTIME_NATIVE;
                        }
                        else if ( stricmp( dxtCompressionMethod, "squish" ) == 0 ||
                                  stricmp( dxtCompressionMethod, "libsquish" ) == 0 )
                        {
                            c_dxtRuntimeType = rw::DXTRUNTIME_SQUISH;
                        }
                    }

                    // Warning level.
                    if ( mainEntry->Find( "warningLevel" ) )
                    {
                        rwEngine->SetWarningLevel( mainEntry->GetInt( "warningLevel" ) );
                    }

                    // Ignore secure warnings.
                    if ( mainEntry->Find( "ignoreSecureWarnings" ) )
                    {
                        bool doIgnore = mainEntry->GetBool( "ignoreSecureWarnings" );

                        rwEngine->SetIgnoreSecureWarnings( doIgnore );
                    }

                    // Reconstruct IMG Archives.
                    if ( mainEntry->Find( "reconstructIMGArchives" ) )
                    {
                        c_reconstructIMGArchives = mainEntry->GetBool( "reconstructIMGArchives" );
                    }

                    // Fix incompatible rasters.
                    if ( mainEntry->Find( "fixIncompatibleRasters" ) )
                    {
                        c_fixIncompatibleRasters = mainEntry->GetBool( "fixIncompatibleRasters" );
                    }

                    // DXT packed decompression.
                    if ( mainEntry->Find( "dxtPackedDecompression" ) )
                    {
                        c_dxtPackedDecompression = mainEntry->GetBool( "dxtPackedDecompression" );
                    }
                    
                    // IMG archive compression
                    if ( mainEntry->Find( "imgArchivesCompressed" ) )
                    {
                        c_imgArchivesCompressed = mainEntry->GetBool( "imgArchivesCompressed" );
                    }

                    // Serialization compatibility setting.
                    if ( mainEntry->Find( "ignoreSerializationRegions" ) )
                    {
                        c_ignoreSerializationRegions = mainEntry->GetBool( "ignoreSerializationRegions" );
                    }

                    // Debug output flag.
                    if ( mainEntry->Find( "outputDebug" ) )
                    {
                        c_outputDebug = mainEntry->GetBool( "outputDebug" );
                    }
                }

                // Kill the configuration.
                delete configFile;
            }

            // Set some configuration.
            rwEngine->SetVersion( rw::KnownVersions::getGameVersion( c_gameVersion ) );
            rwEngine->SetPaletteRuntime( c_palRuntimeType );
            rwEngine->SetDXTRuntime( c_dxtRuntimeType );

            rwEngine->SetFixIncompatibleRasters( c_fixIncompatibleRasters );

            rwEngine->SetIgnoreSerializationBlockRegions( c_ignoreSerializationRegions );

            // Output some debug info.
            std::cout
                <<
                "=========================\n" \
                "Configuration:\n" \
                "=========================\n";

            std::cout
                << "* outputRoot: " << c_outputRoot << std::endl
                << "* gameRoot: " << c_gameRoot << std::endl;

            rw::LibraryVersion targetVersion = rwEngine->GetVersion();

            const char *strTargetVersion = "unknown";

            if ( targetVersion.rwLibMajor == 3 && targetVersion.rwLibMinor == 6 )
            {
                if ( c_gameVersion == rw::KnownVersions::SA )
                {
                    strTargetVersion = "San Andreas";
                }
                else if ( c_gameVersion == rw::KnownVersions::MANHUNT )
                {
                    strTargetVersion = "Manhunt";
                }
            }
            else if ( targetVersion.rwLibMajor == 3 && ( targetVersion.rwLibMinor == 3 || targetVersion.rwLibMinor == 4 ) )
            {
                strTargetVersion = "Vice City";
            }
            else if ( targetVersion.rwLibMajor == 3 && ( targetVersion.rwLibMinor == 0 || targetVersion.rwLibMinor == 1 ) )
            {
                strTargetVersion = "GTA 3";
            }

            std::cout
                << "* targetVersion: " << strTargetVersion << " [rwver: " << targetVersion.toString() << "]" << std::endl;

            const char *strTargetPlatform = "unknown";

            if ( c_targetPlatform == PLATFORM_PC )
            {
                strTargetPlatform = "PC";
            }
            else if ( c_targetPlatform == PLATFORM_PS2 )
            {
                strTargetPlatform = "PS2";
            }
            else if ( c_targetPlatform == PLATFORM_XBOX )
            {
                strTargetPlatform = "XBOX";
            }
            else if ( c_targetPlatform == PLATFORM_DXT_MOBILE )
            {
                strTargetPlatform = "S3TC [mobile]";
            }   
            else if ( c_targetPlatform == PLATFORM_PVR )
            {
                strTargetPlatform = "PowerVR [mobile]";
            }
            else if ( c_targetPlatform == PLATFORM_ATC )
            {
                strTargetPlatform = "ATI [mobile]";
            }
            else if ( c_targetPlatform == PLATFORM_UNC_MOBILE )
            {
                strTargetPlatform = "uncompressed [mobile]";
            }

            std::cout
                << "* targetPlatform: " << strTargetPlatform << std::endl;

            std::cout
                << "* clearMipmaps: " << ( c_clearMipmaps ? "true" : "false" ) << std::endl;

            std::cout
                << "* generateMipmaps: " << ( c_generateMipmaps ? "true" : "false" ) << std::endl;

            const char *mipGenModeString = "unknown";

            if ( c_mipGenMode == rw::MIPMAPGEN_DEFAULT )
            {
                mipGenModeString = "default";
            }
            else if ( c_mipGenMode == rw::MIPMAPGEN_CONTRAST )
            {
                mipGenModeString = "contrast";
            }
            else if ( c_mipGenMode == rw::MIPMAPGEN_BRIGHTEN )
            {
                mipGenModeString = "brighten";
            }
            else if ( c_mipGenMode == rw::MIPMAPGEN_DARKEN )
            {
                mipGenModeString = "darken";
            }
            else if ( c_mipGenMode == rw::MIPMAPGEN_SELECTCLOSE )
            {
                mipGenModeString = "selectclose";
            }

            std::cout
                << "* mipGenMode: " << mipGenModeString << std::endl;

            std::cout
                << "* mipGenMaxLevel: " << c_mipGenMaxLevel << std::endl;

            std::cout
                << "* improveFiltering: " << ( c_improveFiltering ? "true" : "false" ) << std::endl;

            std::cout
                << "* compressTextures: " << ( compressTextures ? "true" : "false" ) << std::endl;

            std::cout
                << "* compressionQuality: " << ( c_compressionQuality ) << std::endl;

            const char *strPalRuntimeType = "unknown";

            if ( c_palRuntimeType == rw::PALRUNTIME_NATIVE )
            {
                strPalRuntimeType = "native";
            }
            else if ( c_palRuntimeType == rw::PALRUNTIME_PNGQUANT )
            {
                strPalRuntimeType = "pngquant";
            }

            std::cout
                << "* palRuntimeType: " << strPalRuntimeType << std::endl;

            const char *strDXTRuntimeType = "unknown";

            if ( c_dxtRuntimeType == rw::DXTRUNTIME_NATIVE )
            {
                strDXTRuntimeType = "native";
            }
            else if ( c_dxtRuntimeType == rw::DXTRUNTIME_SQUISH )
            {
                strDXTRuntimeType = "squish";
            }

            std::cout
                << "* dxtRuntimeType: " << strDXTRuntimeType << std::endl;

            std::cout
                << "* warningLevel: " << rwEngine->GetWarningLevel() << std::endl;

            std::cout
                << "* ignoreSecureWarnings: " << ( rwEngine->GetIgnoreSecureWarnings() ? "true" : "false" ) << std::endl;

            std::cout
                << "* reconstructIMGArchives: " << ( c_reconstructIMGArchives ? "true" : "false" ) << std::endl;

            std::cout
                << "* fixIncompatibleRasters: " << ( c_fixIncompatibleRasters ? "true" : "false" ) << std::endl;

            std::cout
                << "* dxtPackedDecompression: " << ( c_dxtPackedDecompression ? "true" : "false" ) << std::endl;

            std::cout
                << "* imgArchivesCompressed: " << ( c_imgArchivesCompressed ? "true" : "false" ) << std::endl;

            std::cout
                << "* ignoreSerializationRegions: " << ( c_ignoreSerializationRegions ? "true" : "false" ) << std::endl;

            // Finish with a newline.
            std::cout << std::endl;

            // Do the conversion!
            {
                CFileTranslator *absGameRootTranslator = NULL;
                CFileTranslator *absOutputRootTranslator = NULL;

                bool hasGameRoot = obtainAbsolutePath( c_gameRoot.c_str(), absGameRootTranslator, false, true );
                bool hasOutputRoot = obtainAbsolutePath( c_outputRoot.c_str(), absOutputRootTranslator, true, true );

                // Create a debug directory if we want to output debug.
                CFileTranslator *absDebugOutputTranslator = NULL;

                bool hasDebugRoot = false;

                if ( c_outputDebug )
                {
                    hasDebugRoot = obtainAbsolutePath( "debug_output/", absDebugOutputTranslator, true, true );
                }

                if ( hasGameRoot && hasOutputRoot )
                {
                    try
                    {
                        // File roots are prepared.
                        // We can start processing files.
                        gtaFileProcessor <_discFileSentry> fileProc;

                        fileProc.setArchiveReconstruction( c_reconstructIMGArchives );

                        fileProc.setUseCompressedIMGArchives( c_imgArchivesCompressed );

                        _discFileSentry sentry;
                        sentry.targetPlatform = c_targetPlatform;
                        sentry.clearMipmaps = c_clearMipmaps;
                        sentry.generateMipmaps = c_generateMipmaps;
                        sentry.mipGenMode = c_mipGenMode;
                        sentry.mipGenMaxLevel = c_mipGenMaxLevel;
                        sentry.improveFiltering = c_improveFiltering;
                        sentry.doCompress = compressTextures;
                        sentry.compressionQuality = c_compressionQuality;
                        sentry.gameVersion = c_gameVersion;
                        sentry.outputDebug = c_outputDebug;
                        sentry.debugTranslator = absDebugOutputTranslator;

                        fileProc.process( &sentry, absGameRootTranslator, absOutputRootTranslator );

                        // Output any warnings.
                        _warningMan.Purge();
                    }
                    catch( termination_request& )
                    {
                        // OK.
                        std::cout
                            << "terminated application" << std::endl;

                        successful = false;
                    }
                }
                else
                {
                    if ( !hasGameRoot )
                    {
                        std::cout
                            << "could not get a filesystem handle to the game root" << std::endl;
                    }

                    if ( !hasOutputRoot )
                    {
                        std::cout
                            << "could not get a filesystem handle to the output root" << std::endl;
                    }
                }

                // Clean up resources.
                if ( hasDebugRoot )
                {
                    delete absDebugOutputTranslator;
                }

                if ( hasGameRoot )
                {
                    delete absGameRootTranslator;
                }

                if ( hasOutputRoot )
                {
                    delete absOutputRootTranslator;
                }
            }

            // Shutdown environments.
            std::cout << "\ncleaning up...\n";

            CFileSystem::Destroy( fsHandle );
        }
        else
        {
            std::cout << "error: incompatible RenderWare environment.\n";
        }

        // Destroy the RenderWare engine again.
        rw::DeleteEngine( rwEngine );
    }

    return successful;
}

int main( int argc, char *argv[] )
{
    bool success = true;

    // Register an exception handler for easier debugging.
    TxdGenExceptionHandler _exceptHandler;

    DbgTrace::RegisterExceptionHandler( &_exceptHandler );

    // Launch the application.
    success = ApplicationMain();

    // Unregister the exception handler again.
    DbgTrace::UnregisterExceptionHandler( &_exceptHandler );

#ifdef PAGE_HEAP_MEMORY_STATS
    // Check leaked memory.
    DbgHeap_CheckActiveBlocks();
#endif

    _has_terminated = true;

    return ( success ) ? 0 : -1;
}