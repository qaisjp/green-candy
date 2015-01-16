#include "StdInc.h"
#include <fstream>

#include <SharedUtil.hpp>
#include <iostream>
#include <streambuf>

#include <signal.h>

static bool _please_terminate = false;

static bool _has_terminated = false;

struct termination_request
{
};

struct imgImportTraverse
{
    CFileTranslator *srcRoot;
    CFileTranslator *targetRoot;
};

static void _file_copy_iterator( const filePath& absPath, void *ud )
{
    if ( _please_terminate == true )
    {
        throw termination_request();
    }

    imgImportTraverse *info = (imgImportTraverse*)ud;

    // Perform a file copy.
    filePath relPath;

    bool hasRelativePath = info->srcRoot->GetRelativePath( absPath, true, relPath );

    if ( hasRelativePath )
    {
        std::cout << "copying " << relPath.c_str() << std::endl;

        FileSystem::FileCopy( info->srcRoot, relPath.c_str(), info->targetRoot, relPath.c_str() );
    }
}

static void _term_signal( int signalID )
{
    _please_terminate = true;

    while ( _has_terminated == false )
    {
        Sleep( 10000 );
    }
}

int main( int argc, char *argv[] )
{
    signal( SIGTERM, _term_signal );
    signal( SIGBREAK, _term_signal );

    std::cout
        << "IMG file constructor by The_GTA. Compiled on " __DATE__ << std::endl
        << "Use at your own risk!" << std::endl << std::endl
        << "Visit mtasa.com" << std::endl << std::endl;

    bool success = true;

    // Initialize the environment.
    CFileSystem *fsHandle = CFileSystem::Create();

    // Load the configuration.
    CINI *imgConfig = LoadINI( "imgimport.ini" );

    std::string c_sourceRoot = "imgimport/";
    std::string c_imgFilePath = "imgout.img";

    if ( imgConfig )
    {
        if ( CINI::Entry *mainEntry = imgConfig->GetEntry( "Main" ) )
        {
            // Source root.
            if ( const char *strSourceRoot = mainEntry->Get( "sourceRoot" ) )
            {
                c_sourceRoot = strSourceRoot;
            }

            // Img file path.
            if ( const char *strImgFilePath = mainEntry->Get( "imgFile" ) )
            {
                c_imgFilePath = strImgFilePath;
            }
        }
    }

    // Print the configuration.
    printf(
        "=============================\n" \
        "Configuration:\n" \
        "=============================\n"
    );

    std::cout
        << "* sourceRoot: " << c_sourceRoot << std::endl;

    std::cout
        << "* imgFile: " << c_imgFilePath << std::endl;

    // Finish with an endline.
    std::cout << std::endl;

    // Do it.
    {
        // Get the filesystem link to the directory which should have the IMG archive.
        CFileTranslator *absImgFileRoot = NULL;
        CFileTranslator *absSourceRoot = NULL;

        bool hasIMGFileRoot = obtainAbsolutePath(c_imgFilePath.c_str(), absImgFileRoot, true );
        bool hasSourceRoot = obtainAbsolutePath(c_sourceRoot.c_str(), absSourceRoot, false );

        if ( hasIMGFileRoot && hasSourceRoot )
        {
            // Get the name of the archive file.
            std::string fileName = FileSystem::GetFileNameItem( c_imgFilePath.c_str(), true );

            CArchiveTranslator *outputArchive = NULL;

            // Try to find an already existing file.
            // If it exists, we must not create an archive.
            if ( absImgFileRoot->Exists( fileName.c_str() ) )
            {
                std::cout << "opening IMG archive" << std::endl << std::endl;

                outputArchive = fsHandle->OpenIMGArchive( absImgFileRoot, fileName.c_str() );

                if ( !outputArchive )
                {
                    std::cout << "failed to open IMG archive" << std::endl;
                }
            }
            else
            {
                std::cout << "creating IMG archive" << std::endl;

                outputArchive = fsHandle->CreateIMGArchive( absImgFileRoot, fileName.c_str() );

                if ( !outputArchive )
                {
                    std::cout << "failed to create IMG archive" << std::endl;
                }
            }

            if ( outputArchive )
            {
                try
                {
                    // Import all files into the archive and save it.
                    imgImportTraverse trav;
                    trav.srcRoot = absSourceRoot;
                    trav.targetRoot = outputArchive;

                    absSourceRoot->ScanDirectory( "@", "*", true, NULL, _file_copy_iterator, &trav );

                    std::cout << std::endl;

                    std::cout << "writing ...";

                    // Save it.
                    outputArchive->Save();

                    std::cout << "done." << std::endl;

                    // Close it.
                    delete outputArchive;
                }
                catch( termination_request& )
                {
                    delete outputArchive;

                    std::cout << "terminated." << std::endl;
                }
            }
        }
        else
        {
            if ( !hasIMGFileRoot )
            {
                std::cout << "could not get a filesystem link to the IMG file target directory" << std::endl;
            }

            if ( !hasSourceRoot )
            {
                std::cout << "could not get a filesystem link to the source file root" << std::endl;
            }
        }

        // Clean up.
        if ( absImgFileRoot )
        {
            delete absImgFileRoot;
        }

        if ( absSourceRoot )
        {
            delete absSourceRoot;
        }
    }

    // Destroy modules again.
    CFileSystem::Destroy( fsHandle );

    _has_terminated = true;

    return ( success ) ? 0 : -1;
}