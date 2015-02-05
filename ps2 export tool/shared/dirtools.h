template <typename sentryType>
struct gtaFileProcessor
{
    inline gtaFileProcessor( void )
    {
        this->reconstruct_archives = true;
        this->use_compressed_img_archives = true;
    }

    inline ~gtaFileProcessor( void )
    {
        return;
    }

    inline void process( sentryType *theSentry, CFileTranslator *discHandle, CFileTranslator *buildRoot )
    {
        _discFileTraverse traverse;
        traverse.discHandle = discHandle;
        traverse.buildRoot = buildRoot;
        traverse.isInArchive = false;
        traverse.sentry = theSentry;
        traverse.reconstruct_archives = this->reconstruct_archives;
        traverse.use_compressed_img_archives = this->use_compressed_img_archives;

        discHandle->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );
    }

    inline void setArchiveReconstruction( bool doReconstruct )
    {
        this->reconstruct_archives = doReconstruct;
    }

    inline void setUseCompressedIMGArchives( bool doUse )
    {
        this->use_compressed_img_archives = doUse;
    }

private:
    bool reconstruct_archives;
    bool use_compressed_img_archives;

    struct _discFileTraverse
    {
        inline _discFileTraverse( void )
        {
            this->anyWork = false;
        }

        CFileTranslator *discHandle;
        CFileTranslator *buildRoot;
        bool isInArchive;

        bool anyWork;

        bool reconstruct_archives;
        bool use_compressed_img_archives;

        sentryType *sentry;
    };

    static void _discFileCallback( const filePath& discFilePathAbs, void *userdata )
    {
        _discFileTraverse *info = (_discFileTraverse*)userdata;

        bool anyWork = false;

        // Preprocess the file.
        // Create a destination file inside of the build root using the relative path from the source trans root.
        filePath relPathFromRoot;
        
        bool hasTargetRelativePath = info->discHandle->GetRelativePathFromRoot( discFilePathAbs.c_str(), true, relPathFromRoot );

        if ( hasTargetRelativePath )
        {
            bool hasPreprocessedFile = false;

            std::string extention;

            SString fileName = FileSystem::GetFileNameItem( discFilePathAbs.c_str(), false, NULL, &extention );

            if ( extention.size() != 0 )
            {
                if ( stricmp( extention.c_str(), "IMG" ) == 0 )
                {
                    printf( "processing %s ...\n", relPathFromRoot.c_str() );

                    // Open the IMG archive.
                    CIMGArchiveTranslatorHandle *srcIMGRoot = NULL;

                    if ( info->use_compressed_img_archives )
                    {
                        srcIMGRoot = fileSystem->OpenCompressedIMGArchive( info->discHandle, relPathFromRoot.c_str() );
                    }
                    else
                    {
                        srcIMGRoot = fileSystem->OpenIMGArchive( info->discHandle, relPathFromRoot.c_str() );
                    }

                    if ( srcIMGRoot )
                    {
                        try
                        {
                            // If we have found an IMG archive, we perform the same stuff for files inside of it.
                            CFileTranslator *outputRoot = NULL;
                            CArchiveTranslator *outputRoot_archive = NULL;

                            if ( info->reconstruct_archives )
                            {
                                // Grab the version of the source IMG file.
                                // We want to output rebuilt archives in the same version.
                                eIMGArchiveVersion imgVersion = srcIMGRoot->GetVersion();

                                // We copy the files into a new IMG archive tho.
                                CArchiveTranslator *newIMGRoot = NULL;

                                if ( info->use_compressed_img_archives )
                                {
                                    newIMGRoot = fileSystem->CreateCompressedIMGArchive( info->buildRoot, relPathFromRoot.c_str(), imgVersion );
                                }
                                else
                                {
                                    newIMGRoot = fileSystem->CreateIMGArchive( info->buildRoot, relPathFromRoot.c_str(), imgVersion );
                                }

                                if ( newIMGRoot )
                                {
                                    outputRoot = newIMGRoot;
                                    outputRoot_archive = newIMGRoot;
                                }
                            }
                            else
                            {
                                // Create a new directory in place of the archive.
                                filePath srcPathToNewDir;

                                info->discHandle->GetRelativePathFromRoot( discFilePathAbs.c_str(), false, srcPathToNewDir );

                                filePath pathToNewDir;

                                info->buildRoot->GetFullPathFromRoot( srcPathToNewDir.c_str(), false, pathToNewDir );

                                pathToNewDir += fileName.c_str();
                                pathToNewDir += "_archive/";

                                bool createDirSuccess = info->buildRoot->CreateDir( pathToNewDir.c_str() );

                                if ( createDirSuccess )
                                {
                                    CFileTranslator *newDirRoot = fileSystem->CreateTranslator( pathToNewDir.c_str() );

                                    if ( newDirRoot )
                                    {
                                        outputRoot = newDirRoot;
                                    }
                                }
                            }

                            if ( outputRoot )
                            {
                                try
                                {
                                    // Run into shit.
                                    _discFileTraverse traverse;
                                    traverse.discHandle = srcIMGRoot;
                                    traverse.buildRoot = outputRoot;
                                    traverse.isInArchive = ( outputRoot_archive != NULL ) || info->isInArchive;
                                    traverse.sentry = info->sentry;
                                    traverse.reconstruct_archives = info->reconstruct_archives;
                                    traverse.use_compressed_img_archives = info->use_compressed_img_archives;

                                    srcIMGRoot->ScanDirectory( "@", "*", true, NULL, _discFileCallback, &traverse );

                                    if ( outputRoot_archive != NULL )
                                    {
                                        printf( "writing " );

                                        if ( !traverse.anyWork )
                                        {
                                            printf( "(no change)" );
                                        }

                                        printf( "... " );

                                        outputRoot_archive->Save();

                                        printf( "done.\n\n" );

                                        anyWork = true;
                                    }
                                    else
                                    {
                                        printf( "finished.\n\n" );

                                        anyWork = true;
                                    }

                                    hasPreprocessedFile = true;
                                }
                                catch( ... )
                                {
                                    delete outputRoot;

                                    throw;
                                }

                                delete outputRoot;
                            }
                            else
                            {
                                info->sentry->OnArchiveFail( fileName, extention );
                            }
                        }
                        catch( ... )
                        {
                            // On exception, we must clean up after ourselves :)
                            delete srcIMGRoot;

                            throw;
                        }

                        // Clean up.
                        delete srcIMGRoot;
                    }
                    else
                    {
                        printf( "not an IMG archive.\n" );
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
                
                if ( sourceStream )
                {
                    try
                    {
                        // Execute the sentry.
                        bool hasDoneAnyWork = info->sentry->OnSingletonFile( info->discHandle, info->buildRoot, relPathFromRoot, fileName, extention, sourceStream, info->isInArchive );

                        if ( hasDoneAnyWork )
                        {
                            anyWork = true;
                        }
                    }
                    catch( ... )
                    {
                        delete sourceStream;

                        throw;
                    }

                    delete sourceStream;
                }
            }
        }

        if ( anyWork )
        {
            info->anyWork = true;
        }
    }
};

inline bool obtainAbsolutePath( const char *path, CFileTranslator*& transOut, bool createDir, bool hasToBeDirectory = true )
{
    bool hasTranslator = false;
    
    bool newTranslator = false;
    CFileTranslator *translator = NULL;
    filePath thePath;

    // Check whether the file path has a trailing slash.
    // If it has not, then append one.
    filePath inputPath( path );

    if ( hasToBeDirectory )
    {
        if ( !FileSystem::IsPathDirectory( inputPath ) )
        {
            inputPath += '/';
        }
    }

    if ( fileRoot->GetFullPath( inputPath.c_str(), true, thePath ) )
    {
        translator = fileRoot;

        hasTranslator = true;
    }

    if ( !hasTranslator )
    {
        const char *inputPathPtr = inputPath.c_str();

        if ( *inputPathPtr != 0 && *(inputPathPtr+1) == ':' && ( *(inputPathPtr+2) == '/' || *(inputPathPtr+2) == '\\' ) )
        {
            char diskRootDesc[4];
            memcpy( diskRootDesc, inputPathPtr, 3 );

            diskRootDesc[3] = '\0';

            CFileTranslator *diskTranslator = fileSystem->CreateTranslator( diskRootDesc );

            if ( diskTranslator )
            {
                bool canResolve = diskTranslator->GetFullPath( inputPathPtr, true, thePath );

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
        bool canCreateTranslator = false;

        if ( createDir )
        {
            bool createPath = translator->CreateDir( thePath.c_str() );

            if ( createPath )
            {
                canCreateTranslator = true;
            }
        }
        else
        {
            bool existsPath = translator->Exists( thePath.c_str() );

            if ( existsPath )
            {
                canCreateTranslator = true;
            }
        }

        if ( canCreateTranslator )
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