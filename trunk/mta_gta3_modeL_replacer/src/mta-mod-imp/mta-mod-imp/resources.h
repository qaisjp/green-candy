#ifndef _RESOURCES_
#define _RESOURCES_

template <typename dispatchType>
struct ResourceManager
{
    dispatchType& dispatch;

    CFileTranslator *outputRoot;
    CFileTranslator *resourceRoot;

    const char *txdNames[65536];
    const char *txdName;
    const char *colName;
    unsigned short usTxdNames;

    inline ResourceManager( int a, dispatchType& _dispatch ) : dispatch( _dispatch )
    {
        this->outputRoot = AcquireOutputRoot();
        this->resourceRoot = AcquireResourceRoot();

        usTxdNames = 0;

        txdName = NULL;
        colName = NULL;
    }

    inline bool ResourceExists( const char *name )
    {
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
	    unsigned int k;

	    // Copy the model file
	    _snprintf(buffer, 1023, "%s.dff", name);

	    if ( !ResourceExists(buffer) )
	    {
		    printf("error: model missing (%s)\n", buffer);
		    return false;
	    }

	    _snprintf(copyBuffer, 1023, "models\\%s.dff", name);

	    if ( _CopyOnlyIfRequired(buffer, copyBuffer) )
		    printf("copying model '%s'\n", name);

	    if ( dispatch.requireCollision( name, lod ) )
	    {
		    // Now the collision
		    _snprintf(buffer, 1023, "%s.col", name);

		    if ( !ResourceExists(buffer) )
		    {
			    if ( !lod )
			    {
				    printf("error: collision missing (%s)\n", buffer);
				    return false;
			    }

			    colName = NULL;
		    }
		    else
		    {
			    colName = name;

			    _snprintf(copyBuffer, 1023, "coll\\%s.col", colName);

			    if (_CopyOnlyIfRequired(buffer, copyBuffer))
				    printf("copying collision '%s'\n", colName);
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

	    for (k=0; k < usTxdNames; k++)
		    if (strcmp(txdNames[k], txdName) == 0)
			    break;

	    // Little hack
	    if (k == usTxdNames)
	    {
		    // Copy over resources
		    _snprintf(buffer, 1023, "%s.txd", txdName);

		    if (!ResourceExists(buffer))
			    printf("texture missing: %s (ignoring)\n", buffer);
		    else
		    {
			    _snprintf(copyBuffer, 1023, "textures\\%s.txd", txdName);

			    // Copy the resource over
			    if (_CopyOnlyIfRequired(buffer, copyBuffer))
				    printf("copying texture '%s'\n", txdName);

                dispatch.OnTxdEntry( txdName );
		    }

		    txdNames[usTxdNames++] = txdName;
	    }

	    names[usNames++] = name;

        dispatch.OnModelEntry( name );

	    if ( colName )
	    {
            dispatch.OnCOLEntry( colName );
	    }

	    return true;
    }
};

#endif //_RESOURCES_