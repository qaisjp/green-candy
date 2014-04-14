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

#endif //_RESOURCES_
