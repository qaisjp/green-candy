#include "StdInc.h"

#include <renderware.h>
#include <fstream>

#include <SharedUtil.hpp>
#include <iostream>
#include <streambuf>

#include <signal.h>

static CFileSystem *fsHandle = NULL;

static bool _please_terminate = false;

struct termination_request
{
};

enum eTargetPlatform
{
    PLATFORM_PC,
    PLATFORM_PS2,
    PLATFORM_XBOX
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

static bool ProcessTXDArchive( CFileTranslator *srcRoot, CFile *srcStream, CFile *targetStream, eTargetPlatform targetPlatform, bool doCompress, float compressionQuality, rw::KnownVersions::eGameVersion gameVersion, std::string& errMsg )
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
        catch( rw::RwException& except )
        {
            errMsg = "error reading txd: " + except.message;
            return false;
        }
    }

    // Process all textures.
    bool processSuccessful = true;

    try
    {
        for ( unsigned int n = 0; n < txd.texList.size(); n++ )
        {
            rw::NativeTexture& tex = txd.texList[n];

            // Convert the texture to Direct3D 9 format.
            bool isPrepared = tex.convertToDirect3D9();

            // If the texture is prepared, do whatever.
            if ( isPrepared )
            {   
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

                // Convert it into the target platform.
                if ( targetPlatform == PLATFORM_PS2 )
                {
                    tex.convertToPS2();
                }
                else if ( targetPlatform == PLATFORM_XBOX )
                {
                    tex.convertToXbox();
                }
                else if ( targetPlatform == PLATFORM_PC )
                {
                    // Make sure that we have a D3DFORMAT field.
                    if ( !tex.isDirect3DWritable() )
                    {
                        tex.makeDirect3DCompatible();
                    }

                    // Depends on the game.
                    if (gameVersion == rw::KnownVersions::SA)
                    {
                        tex.convertToDirect3D9();
                    }
                    else
                    {
                        tex.convertToDirect3D8();
                    }
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
        return false;
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
            }
            catch( rw::RwException& except )
            {
                errMsg = "error writing txd: " + except.message;
                return false;
            }

            hasProcessed = true;
        }
    }

    return hasProcessed;
}

struct _discFileSentry
{
    eTargetPlatform targetPlatform;
    bool doCompress;
    float compressionQuality;
    rw::KnownVersions::eGameVersion gameVersion;

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

                    bool couldProcessTXD = ProcessTXDArchive( sourceRoot, sourceStream, targetStream, this->targetPlatform, this->doCompress, this->compressionQuality, this->gameVersion, errorMessage );

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

bool ApplicationMain( void )
{
    signal( SIGTERM, _term_handler );
    signal( SIGBREAK, _term_handler );

    std::cout <<
        "RenderWare TXD generator tool by The_GTA. Compiled on " __DATE__ "\n" \
        "Use this tool at your own risk!\n\n" \
        "Visit mtasa.com\n\n";

    bool successful = true;

    // Initialize environments.
    fsHandle = CFileSystem::Create();
    
    // By default, we create San Andreas files.
    rw::KnownVersions::eGameVersion c_gameVersion = rw::KnownVersions::SA;

    // Set up the warning buffer.
    rw::rwInterface.SetWarningManager( &_warningMan );

    // Read the configuration file.
    CINI *configFile = LoadINI( "txdgen.ini" );

    std::string c_outputRoot = "txdgen_out/";
    std::string c_gameRoot = "txdgen/";

    eTargetPlatform c_targetPlatform = PLATFORM_PC;

    bool compressTextures = false;

    rw::ePaletteRuntimeType c_palRuntimeType = rw::PALRUNTIME_NATIVE;

    rw::eDXTCompressionMethod c_dxtRuntimeType = rw::DXTRUNTIME_NATIVE;

    bool c_reconstructIMGArchives = true;

    bool c_fixIncompatibleRasters = true;
    bool c_dxtPackedDecompression = false;

    bool c_imgArchivesCompressed = false;

    float c_compressionQuality = 0.5f;

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
                          stricmp( targetPlatform, "Playstation 2" ) == 0 )
                {
                    c_targetPlatform = PLATFORM_PS2;
                }
                else if ( stricmp( targetPlatform, "XBOX" ) == 0 )
                {
                    c_targetPlatform = PLATFORM_XBOX;
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
                
                if ( hasGameVer )
                {
                    c_gameVersion = gameVer;
                }
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
                rw::rwInterface.SetWarningLevel( mainEntry->GetInt( "warningLevel" ) );
            }

            // Ignore secure warnings.
            if ( mainEntry->Find( "ignoreSecureWarnings" ) )
            {
                bool doIgnore = mainEntry->GetBool( "ignoreSecureWarnings" );

                rw::rwInterface.SetIgnoreSecureWarnings( doIgnore );
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
        }

        // Kill the configuration.
        delete configFile;
    }

    // Set some configuration.
    rw::rwInterface.SetVersion( rw::KnownVersions::getGameVersion( c_gameVersion ) );
    rw::rwInterface.SetPaletteRuntime( c_palRuntimeType );
    rw::rwInterface.SetDXTRuntime( c_dxtRuntimeType );

    rw::rwInterface.SetFixIncompatibleRasters( c_fixIncompatibleRasters );

    // Output some debug info.
    std::cout
        <<
        "=========================\n" \
        "Configuration:\n" \
        "=========================\n";

    std::cout
        << "* outputRoot: " << c_outputRoot << std::endl
        << "* gameRoot: " << c_gameRoot << std::endl;

    rw::LibraryVersion targetVersion = rw::rwInterface.GetVersion();

    const char *strTargetVersion = "unknown";

    if ( targetVersion.rwLibMajor == 3 && targetVersion.rwLibMinor == 6 )
    {
        strTargetVersion = "San Andreas";
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

    std::cout
        << "* targetPlatform: " << strTargetPlatform << std::endl;

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
        << "* warningLevel: " << rw::rwInterface.GetWarningLevel() << std::endl;

    std::cout
        << "* ignoreSecureWarnings: " << ( rw::rwInterface.GetIgnoreSecureWarnings() ? "true" : "false" ) << std::endl;

    std::cout
        << "* reconstructIMGArchives: " << ( c_reconstructIMGArchives ? "true" : "false" ) << std::endl;

    std::cout
        << "* fixIncompatibleRasters: " << ( c_fixIncompatibleRasters ? "true" : "false" ) << std::endl;

    std::cout
        << "* dxtPackedDecompression: " << ( c_dxtPackedDecompression ? "true" : "false" ) << std::endl;

    std::cout
        << "* imgArchivesCompressed: " << ( c_imgArchivesCompressed ? "true" : "false" ) << std::endl;

    // Finish with a newline.
    std::cout << std::endl;

    // Do the conversion!
    {
        CFileTranslator *absGameRootTranslator = NULL;
        CFileTranslator *absOutputRootTranslator = NULL;

        bool hasGameRoot = obtainAbsolutePath( c_gameRoot.c_str(), absGameRootTranslator, false, true );
        bool hasOutputRoot = obtainAbsolutePath( c_outputRoot.c_str(), absOutputRootTranslator, true, true );

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
                sentry.doCompress = compressTextures;
                sentry.compressionQuality = c_compressionQuality;
                sentry.gameVersion = c_gameVersion;

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

    _has_terminated = true;

    return ( success ) ? 0 : -1;
}