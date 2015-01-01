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

static bool ProcessTXDArchive( CFileTranslator *srcRoot, CFile *srcStream, CFile *targetStream, eTargetPlatform targetPlatform, bool doCompress, std::string& errMsg )
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

            bool isPrepared = false;

            // Convert the texture to Direct3D format.
            if ( tex.platform == rw::PLATFORM_PS2 )
            {
                tex.convertFromPS2();

                isPrepared = true;
            }
            else if ( tex.platform == rw::PLATFORM_XBOX )
            {
                tex.convertFromXbox();

                isPrepared = true;
            }
            else if ( tex.platform == rw::PLATFORM_D3D8 || tex.platform == rw::PLATFORM_D3D9 )
            {
                // no conversion necessary.
                isPrepared = true;
            }

            // If the texture is prepared, do whatever.
            if ( isPrepared )
            {
                // Palettize the texture to save space.
                if ( doCompress )
                {
                    if (tex.platformData->getPaletteType() == rw::PALETTE_NONE)
                    {
                        tex.convertToPalette( rw::PALETTE_8BIT );
                    }
                }

                // Convert it into the target platform.
                if ( targetPlatform == PLATFORM_PS2 )
                {
                    tex.convertToPS2();
                }
                else if ( targetPlatform == PLATFORM_XBOX )
                {
                    // todo.
                    assert( 0 );
                }
                else if ( targetPlatform == PLATFORM_PC )
                {
                    // We are already at the appropriate format.
                }
            }
        }
    }
    catch( rw::RwException& except )
    {
        errMsg = "error precessing textures: " + except.message;
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

                    bool couldProcessTXD = ProcessTXDArchive( sourceRoot, sourceStream, targetStream, this->targetPlatform, this->doCompress, errorMessage );

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

inline bool obtainAbsolutePath( const char *path, CFileTranslator*& transOut )
{
    bool hasTranslator = false;
    
    bool newTranslator = false;
    CFileTranslator *translator = NULL;
    filePath thePath;

    if ( fileRoot->GetFullPath( path, true, thePath ) )
    {
        translator = fileRoot;

        hasTranslator = true;
    }

    if ( !hasTranslator )
    {
        if ( *path != 0 && *(path+1) == ':' && *(path+2) == '/' )
        {
            char diskRootDesc[4];
            memcpy( diskRootDesc, path, 3 );

            diskRootDesc[3] = '\0';

            CFileTranslator *diskTranslator = fileSystem->CreateTranslator( diskRootDesc );

            if ( diskTranslator )
            {
                bool canResolve = diskTranslator->GetFullPath( path, true, thePath );

                if ( canResolve )
                {
                    newTranslator = true;
                    translator = diskTranslator;

                    hasTranslator = true;
                }

                if ( !hasTranslator )
                {
                    delete diskTranslator;
                }
            }
        }
    }

    bool success = false;

    if ( hasTranslator )
    {
        bool createPath = translator->CreateDir( thePath.c_str() );

        if ( createPath )
        {
            CFileTranslator *actualRoot = fileSystem->CreateTranslator( thePath.c_str() );

            if ( actualRoot )
            {
                success = true;

                transOut = actualRoot;
            }
        }

        if ( newTranslator )
        {
            delete translator;
        }
    }

    return success;
}

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
    rw::rwInterface.SetVersion( rw::SA );

    // Set up the warning buffer.
    rw::rwInterface.SetWarningManager( &_warningMan );

    // Read the configuration file.
    CINI *configFile = LoadINI( "txdgen.ini" );

    std::string c_outputRoot = "txdgen_out/";
    std::string c_gameRoot = "txdgen/";

    eTargetPlatform c_targetPlatform = PLATFORM_PC;

    bool compressTextures = false;

    rw::ePaletteRuntimeType c_palRuntimeType = rw::PALRUNTIME_NATIVE;

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
                if ( stricmp( targetVersion, "SA" ) == 0 ||
                     stricmp( targetVersion, "SanAndreas" ) == 0 ||
                     stricmp( targetVersion, "San Andreas" ) == 0 )
                {
                    rw::rwInterface.SetVersion( rw::SA );
                }
                else if ( stricmp( targetVersion, "VC" ) == 0 ||
                          stricmp( targetVersion, "ViceCity" ) == 0 ||
                          stricmp( targetVersion, "Vice City" ) == 0 )
                {
                    if ( c_targetPlatform == PLATFORM_PS2 )
                    {
                        rw::rwInterface.SetVersion( rw::VCPS2 );
                    }
                    else
                    {
                        rw::rwInterface.SetVersion( rw::VCPC );
                    }
                }
                else if ( stricmp( targetVersion, "GTAIII" ) == 0 ||
                          stricmp( targetVersion, "III" ) == 0 ||
                          stricmp( targetVersion, "GTA3" ) == 0 ||
                          stricmp( targetVersion, "GTA 3" ) == 0 )
                {
                    rw::rwInterface.SetVersion( rw::GTA3_1 );
                }
            }

            // Compression.
            if ( mainEntry->Find( "compressTextures" ) )
            {
                compressTextures = mainEntry->GetBool( "compressTextures" );
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
        }

        // Kill the configuration.
        delete configFile;
    }

    // Set some configuration.
    rw::rwInterface.SetPaletteRuntime( c_palRuntimeType );

    // Output some debug info.
    std::cout
        <<
        "=========================\n" \
        "Configuration:\n" \
        "=========================\n";

    std::cout
        << "* outputRoot: " << c_outputRoot << std::endl
        << "* gameRoot: " << c_gameRoot << std::endl;

    uint32 targetVersion = rw::rwInterface.GetVersion();

    const char *strTargetVersion = "unknown";

    if ( targetVersion == rw::SA )
    {
        strTargetVersion = "San Andreas";
    }
    else if ( targetVersion == rw::VCPC ||
              targetVersion == rw::VCPS2 )
    {
        strTargetVersion = "Vice City";
    }
    else if ( targetVersion == rw::GTA3_1 || targetVersion == rw::GTA3_2 ||
              targetVersion == rw::GTA3_3 || targetVersion == rw::GTA3_4 )
    {
        strTargetVersion = "GTA 3";
    }

    std::cout
        << "* targetVersion: " << strTargetVersion << std::endl;

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

    // Finish with a newline.
    std::cout << std::endl;

    // Do the conversion!
    {
        CFileTranslator *absGameRootTranslator = NULL;
        CFileTranslator *absOutputRootTranslator = NULL;

        bool hasGameRoot = obtainAbsolutePath( c_gameRoot.c_str(), absGameRootTranslator );
        bool hasOutputRoot = obtainAbsolutePath( c_outputRoot.c_str(), absOutputRootTranslator );

        if ( hasGameRoot && hasOutputRoot )
        {
            try
            {
                // File roots are prepared.
                // We can start processing files.
                gtaFileProcessor <_discFileSentry> fileProc;

                _discFileSentry sentry;
                sentry.targetPlatform = c_targetPlatform;
                sentry.doCompress = compressTextures;

                fileProc.process( &sentry, absGameRootTranslator, absOutputRootTranslator );

                // Output any warnings.
                _warningMan.Purge();
            }
            catch( termination_request& )
            {
                // OK.
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

    success = ApplicationMain();

    _has_terminated = true;

    return ( success ) ? 0 : -1;
}