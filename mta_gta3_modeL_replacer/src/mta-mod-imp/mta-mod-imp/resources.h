#ifndef _RESOURCES_
#define _RESOURCES_

template <typename dispatchType>
struct ResourceManager
{
    dispatchType& dispatch;

    CFileTranslator *outputRoot;
    CFileTranslator *resourceRoot;

    NameRegistry usedTxdNames;
    const char *txdName;
    const char *colName;
    char lodBuffer[128];

    inline ResourceManager( int a, dispatchType& _dispatch ) : dispatch( _dispatch )
    {
        this->outputRoot = AcquireOutputRoot();
        this->resourceRoot = AcquireResourceRoot();

        txdName = NULL;
        colName = NULL;
    }

    inline CFile* OpenOutputFile( const char *filePath, const char *mode )
    {
        CFile *outputStream = outputRoot->Open( filePath, mode );

        if ( !outputStream )
        {
            // Warn the implementation that something failed to create or open.
            printf( "warning: failed to create file handle to '%s' (using mode '%s')\n", filePath, mode );
        }

        return outputStream;
    }

    inline bool ResourceExists( const char *name, const char *outputName )
    {
        if ( outputRoot->Exists( outputName ) )
            return true;

        if ( !resourceRoot )
            return false;

        return resourceRoot->Exists( name );
    }

    inline bool _CopyOnlyIfRequired( const char *src, const char *dst )
    {
        if ( !resourceRoot )
            return false;

        if ( outputRoot->Exists( dst ) )
            return false;

        return FileSystem::FileCopy( resourceRoot, src, outputRoot, dst );
    }

    inline bool AllocateResources( const char *name, bool lod )
    {
	    char buffer[1024];
	    char copyBuffer[1024];

	    // Copy the model file
	    _snprintf(buffer, 1023, "%s.dff", name);
	    _snprintf(copyBuffer, 1023, "models\\%s.dff", name);

	    if ( !ResourceExists(buffer, copyBuffer) )
	    {
		    printf("error: model missing (%s)\n", buffer);
		    return false;
	    }

	    if ( _CopyOnlyIfRequired(buffer, copyBuffer) )
		    printf("copying model '%s'\n", name);

	    if ( dispatch.requireCollision( name, lod ) )
	    {
		    // Now the collision
            const Collision *colEntry = colRegistry->FindCollisionByModel( name );

		    _snprintf(buffer, 1023, "%s.col", name);
		    _snprintf(copyBuffer, 1023, "coll\\%s.col", name);

		    if ( !colEntry && !outputRoot->Exists( copyBuffer ) )
		    {
			    if ( !lod )
			    {
				    printf("error: collision missing (%s)\n", buffer);
				    return false;
			    }

			    colName = NULL;
		    }
		    else if ( colEntry )
		    {
			    colName = name;

			    // Write collision data.
                CFile *outputStream = outputRoot->Open( copyBuffer, "wb" );

                if ( outputStream )
                {
                    colEntry->WriteToFile( outputStream );

                    delete outputStream;
                }
		    }
	    }
	    else
		    colName = NULL;

	    CObject *txdObj = GetObjectByModel(name);

	    if (!txdObj)
	    {
		    printf("could not find object def for '%s'\n", name);
		    return false;
	    }

	    if (lodSupport)
		    _snprintf(lodBuffer, 127, "%.0f", txdObj->m_drawDistance);
	    else
		    strcpy(lodBuffer, "500");

	    txdName = txdObj->m_textureName;

	    // Little hack
	    if ( !usedTxdNames.Exists( txdName ) )
	    {
		    // Copy over resources
		    _snprintf(buffer, 1023, "%s.txd", txdName);
		    _snprintf(copyBuffer, 1023, "textures\\%s.txd", txdName);

		    if (!ResourceExists(buffer, copyBuffer))
			    printf("texture missing: %s (ignoring)\n", buffer);
		    else
		    {
			    // Copy the resource over
			    if (_CopyOnlyIfRequired(buffer, copyBuffer))
				    printf("copying texture '%s'\n", txdName);

                dispatch.OnTxdEntry( txdName );
		    }

		    usedTxdNames.Add( txdName );
	    }

	    g_usedModelNames.Add( name );

        dispatch.OnModelEntry( name );

	    if ( colName )
	    {
            dispatch.OnCOLEntry( colName );
	    }

	    return true;
    }
};

template <typename processorType>
struct InstanceMapper
{
    inline InstanceMapper( processorType& proc ) : processor( proc )
    {
        processor.OnBegin();
    }

    inline ~InstanceMapper( void )
    {
        processor.OnEnd();
    }

    inline void MapInstance( const char *instName, unsigned int modelIndex, double posX, double posY, double posZ, double eulerX, double eulerY, double eulerZ )
    {
        processor.OnInstance( instName, modelIndex, posX + mapXoffset, posY + mapYoffset, posZ + mapZoffset, eulerX, eulerY, eulerZ );
    }

    processorType& processor;
};

struct InstanceProcessorDesc
{
    inline InstanceProcessorDesc( void )
    {
        mapHeader = NULL;
        mapEntry = NULL;
        mapEnd = NULL;
    }

    typedef void (*mapEntryCallback)( CFile *file, const char *instName, unsigned int modelIndex, double posX, double posY, double posZ, double eulerX, double eulerY, double eulerZ );

    const char *mapHeader;
    mapEntryCallback mapEntry;
    const char *mapEnd;
};

struct InstanceProcessor
{
    CFile *mapFile;
    InstanceProcessorDesc instDesc;

    inline InstanceProcessor( CFile *mapFile, InstanceProcessorDesc desc )
    {
        this->mapFile = mapFile;
        this->instDesc = desc;
    }

    inline void OnBegin( void )
    {
        if ( !mapFile )
            return;

        if ( instDesc.mapHeader )
        {
            mapFile->Printf( "%s", instDesc.mapHeader );
        }
    }

    inline void OnInstance( const char *instName, unsigned int modelIndex, double posX, double posY, double posZ, double eulerX, double eulerY, double eulerZ )
    {
        if ( !mapFile )
            return;

        if ( instDesc.mapEntry )
        {
            instDesc.mapEntry( mapFile, instName, modelIndex, posX, posY, posZ, eulerX, eulerY, eulerZ );
        }
    }

    inline void OnEnd( void )
    {
        if ( !mapFile )
            return;

        if ( instDesc.mapEnd )
        {
            mapFile->Printf( "%s", instDesc.mapEnd );
        }
    }
};

#endif //_RESOURCES_
