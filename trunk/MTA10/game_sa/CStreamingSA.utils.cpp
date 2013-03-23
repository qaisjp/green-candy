/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.utils.cpp
*  PURPOSE:     Data streamer utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

// In this file includes examples of clever goto usage.

extern CBaseModelInfoSAInterface **ppModelInfo;
static RtDictSchema *const animDict =   (RtDictSchema*)0x008DED50;
static CModelLoadInfoSA *const VAR_ModelLoadInfo = (CModelLoadInfoSA*)0x008E4CC0;

static streamingLoadCallback_t  streamingLoadCallback = NULL;

/*
    Texture Scanner Namespaces

    Those namesspaces are texture scanners which - once applied, are called
    during RwFindTexture. They are meant to be stack-based, so that they have
    to be unattached the same order they were applied. If the scanner does not
    find the texture in it's environment, it calls the previously attached
    scanner. Most of the time they use the local reference storage.
*/

/*=========================================================
    RwRemapScan

    This logic scans the general VEHICLE.TXD that is
    loaded in _VehicleModels_Init. If the texture was not found,
    the previously applied handler is called.

    It is used during the loading of vehicle models, currently
    GTA:SA internal only. The original GTA:SA function did not
    call the previous texture scanner.
=========================================================*/
namespace RwRemapScan
{
    static RwScanTexDictionaryStackRef_t    prevStackScan;

    // Method which scans VEHICLE.TXD
    // Binary offsets: (1.0 US and 1.0 EU): 0x004C7510
    RwTexture* scanMethod( const char *name )
    {
        RwTexture *tex = g_vehicleTxd->FindNamedTexture( name );

        if ( tex )
            return tex;

        // The_GTA: usually the engine flagged used remap textures with a '#'
        // * It was done by replacing the first character in their name.
        // * The engine would always perform two scans: one for the provided name
        // * and another for the '#' flagged version.
        // We do not want this feature.
        return prevStackScan( name );
    }

    // Stores the previous texture scanner and applies ours
    // Binary offsets: (1.0 US and 1.0 EU): 0x004C75A0
    void Apply()
    {
        prevStackScan = pRwInterface->m_textureManager.m_findInstanceRef;
        pRwInterface->m_textureManager.m_findInstanceRef = scanMethod;
    }

    // Restores the previous texture scanner
    // Binary offsets: (1.0 US and 1.0 EU): 0x004C75C0
    void Unapply()
    {
        pRwInterface->m_textureManager.m_findInstanceRef = prevStackScan;
        prevStackScan = NULL;
    }
};

/*=========================================================
    RwImportedScan (MTA extension)

    This logic scans the textures which virtually included themselves
    to scanning of a specific TXD id slot. It is meant to
    be a cleaner solution than modifying the GTA:SA internal
    TexDictionaries.
    1) GTA:SA can unload it's TXDs without corrupting MTA data (vid memory saved)
    2) Individual textures can be applied instead of whole TXDs (flexibility)
    3) Texture instances come straight - without copying - from the
       provider (i.e. deathmatch Lua). Modifications to the provider
       texture instance result in direct changes of the ingame representation
       (i.e. anisotropy change or filtering flags) (integrity).

    We are applying this whenever a model is loaded, either by the
    GTA:SA streaming requester or the MTA model loader. That is why
    this namespace is globally exported.

    The problem with the current approach is that if a user chooses to replace
    textures, only textures from a model are replaced (i.e. no HUD ones).
    A proper fix would be a hook on the GTA:SA function TXDSetCurrent and
    RwFindTexture. Another interesting fix may be to rewrite all GTA:SA functions
    which use TXDSetCurrent.
=========================================================*/
namespace RwImportedScan
{
    static RwScanTexDictionaryStackRef_t    prevStackScan;
    static unsigned short   txdId;
    static bool             applied;

    static RwTexture* scanMethod( const char *name )
    {
        dictImportList_t& imported = g_dictImports[txdId];

        for ( dictImportList_t::const_iterator iter = imported.begin(); iter != imported.end(); iter++ )
        {
            if ( stricmp( (*iter)->GetName(), name ) == 0 )
                return (*iter)->GetTexture();
        }

        return prevStackScan( name );
    }

    void Apply( unsigned short id )
    {
        if ( !g_dictImports[id].empty() )
        {
            prevStackScan = pRwInterface->m_textureManager.m_findInstanceRef;
            pRwInterface->m_textureManager.m_findInstanceRef = scanMethod;

            txdId = id;
            applied = true;
        }
        else
            applied = false;
    }

    void Unapply()
    {
        if ( applied )
        {
            pRwInterface->m_textureManager.m_findInstanceRef = prevStackScan;
            prevStackScan = NULL;
        }
    }
};

/*=========================================================
    _RpGeometryAllocateNormals (MTA extension)

    Arguments:
        geom - geometry to which you want to attach normals
        mesh - 3d vertice data which requires normals and belongs to geom
    Purpose:
        Calculates "vertex normals" for a mesh from a given geometry.
        They are required to apply position dependent (local) lighting
        to an atomic. The normals array is returned.
=========================================================*/
static inline CVector* _RpGeometryAllocateNormals( RpGeometry *geom, RpGeomMesh *mesh )
{
    CVector *normals = (CVector*)RwAllocAligned( sizeof(CVector) * geom->m_verticeSize, 0x10 );

    for ( unsigned int n = 0; n < geom->m_verticeSize; n++ )
    {
        CVector& norm = normals[n];
        norm.fX = 0; norm.fY = 0; norm.fZ = 0;

        for ( unsigned int i = 0; i < geom->m_triangleSize; i++ )
        {
            const RpTriangle& tri = geom->m_triangles[i];

            if ( tri.v1 == n || tri.v2 == n || tri.v3 == n )
            {
                const CVector& origin = mesh->m_positions[tri.v1];
                CVector v1 = mesh->m_positions[tri.v2] - origin;
                const CVector v2 = mesh->m_positions[tri.v3] - origin;
                v1.CrossProduct( v2 );
                v1.Normalize();

                norm += v1;
            }
        }

        norm.Normalize();
    }

    return normals;
}

/*=========================================================
    _initAtomScene (MTA extension)

    Arguments:
        atom - atomic which should be included into a scene
    Purpose:
        Includes the given atomic into the default GTA:SA scene.
        Then it recalculates the normals for all its meshes.
        By that dynamic lighting will be enabled.
=========================================================*/
static void _initAtomScene( RpAtomic *atom )
{
    atom->m_scene = *p_gtaScene;

    RpGeometry& geom = *atom->m_geometry;
    geom.flags |= RW_GEOMETRY_GLOBALLIGHT;  // apply environmental and directional lights

    // TODO: reenable this using multi-threading (streamline extension!)
    return;

    if ( !( geom.flags & RW_GEOMETRY_NORMALS ) )
    {
        // Allocate normals for every mesh
        for ( unsigned int n = 0; n < geom.m_numMeshes; n++ )
        {
            RpGeomMesh& mesh = geom.m_meshes[n];

            if ( !mesh.m_normals )
                mesh.m_normals = _RpGeometryAllocateNormals( &geom, &mesh );
        }

        if ( !geom.m_skeleton )
            geom.flags |= RW_GEOMETRY_NO_SKIN;

        geom.flags |= RW_GEOMETRY_NORMALS;
    }
}

/*=========================================================
    RpClumpAtomicActivator

    Arguments:
        atom - atomic to set as model for an atomic model info
        replacedId - id of the atomic model info
    Purpose:
        Loads an atomic model info with the given atomic. The atomic
        is registered as the official representative of that model info.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00537150 (FUNC_AtomicsReplacer)
=========================================================*/
static void RpClumpAtomicActivator( RpAtomic *atom, unsigned int replacerId )
{
    CAtomicModelInfoSA *atomInfo = ppModelInfo[replacerId]->GetAtomicModelInfo();
    bool unk;
    char unk2[24];

    // This possibly adds the reference to the texture, we should reven this
    ((void (__cdecl*)(const char*, char*, bool&))0x005370A0)( atom->m_parent->m_nodeName, unk2, unk );

    atom->SetRenderCallback( NULL );

    _initAtomScene( atom );

    if ( unk )
        atomInfo->GetDamageAtomicModelInfo()->SetupPipeline( atom );
    else
        atomInfo->SetAtomic( atom );

    atom->RemoveFromClump();

    atom->AddToFrame( RwFrameCreate() );
    
    atom->m_modelId = replacerId;
}

/*=========================================================
    _initClumpScene (MTA extension)

    Arguments:
        clump - RpClump whose atomics should be included into the scene
    Purpose:
        Prepares a clump for usage with the dynamic lighting system.
        This means that its atomics are added to the default GTA:SA
        scene. Atomics have to belong to a scene so that they traverse
        along sectors. Sectors also contain the lights, so lights are
        properly adjusted.
=========================================================*/
inline static void _initClumpScene( RpClump *clump )
{
    LIST_FOREACH_BEGIN( RpAtomic, clump->m_atomics.root, m_atomics )
        _initAtomScene( item );
    LIST_FOREACH_END
}

/*=========================================================
    LoadClumpFile

    Arguments:
        stream - binary stream which contains the clump file
        model - id of a atomic model info
    Purpose:
        Loads a clump file from the stream. The RpClump is expected
        to have one atomic. For every atomic it calls
        RpClumpAtomicActivator which removes it from the clump and
        adds it to the atomic model info. More than one atomic may
        be applied to the model info if one parent frame name designates
        a damage atomic model info and the other does not.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005371F0
=========================================================*/
static bool __cdecl LoadClumpFile( RwStream *stream, unsigned int model )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[model];
    bool appliedRemapCheck, result;

    CAtomicModelInfoSA *atomInfo = info->GetAtomicModelInfo();

    // MTA extension: Apply our global imports
    RwImportedScan::Apply( info->m_textureDictionary );

    if ( atomInfo && ( atomInfo->m_collFlags & COLL_WETROADREFLECT ) )
    {
        RwRemapScan::Apply();
        appliedRemapCheck = true;
    }
    else
        appliedRemapCheck = false;

    result = false;

    if ( RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
    {
        if ( RpClump *clump = RpClumpStreamRead( stream ) )
        {
            while ( !LIST_EMPTY( clump->m_atomics.root ) )
                RpClumpAtomicActivator( LIST_GETITEM( RpAtomic, clump->m_atomics.root.next, m_atomics ), model );

            RpClumpDestroy( clump );

            result = atomInfo->GetRwObject() != NULL;
        }
    }

    if ( appliedRemapCheck )
        RwRemapScan::Unapply();

    RwImportedScan::Unapply();

    return result;
}

/*=========================================================
    LoadClumpFilePersistent

    Arguments:
        stream - binary stream which contains the clump file
        id - clump model info id
    Purpose:
        Takes the stream and loads either a single or a multi clump
        into the given clump model info. Returns true if there
        was no error during loading.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005372D0
=========================================================*/
static bool __cdecl LoadClumpFilePersistent( RwStream *stream, unsigned int id )
{
    CClumpModelInfoSAInterface *info = (CClumpModelInfoSAInterface*)ppModelInfo[id];

    // Not sure about this flag anymore. Apparently it stands for multi-clump here.
    if ( info->m_renderFlags & RENDER_NOSKELETON )
    {
        RpClump *clump = RpClumpCreate();
        RwFrame *frame = clump->m_parent = RwFrameCreate();

        RwImportedScan::Apply( info->m_textureDictionary );

        while ( RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
        {
            RpClump *item = RpClumpStreamRead( stream );

            if ( !item )
            {
                // Small memory leak fix
                RwFrameDestroy( frame );
                clump->m_parent = NULL;

                RwImportedScan::Unapply();

                RpClumpDestroy( clump );
                return false;
            }
    
            RwFrame *clonedParent = item->m_parent->CloneRecursive();

            frame->Link( clonedParent );
            
            while ( !LIST_EMPTY( item->m_atomics.root ) )
            {
                RpAtomic *atom = LIST_GETITEM( RpAtomic, item->m_atomics.root.next, m_atomics );

                atom->AddToFrame( atom->m_parent->m_root );
                atom->AddToClump( clump );

                _initAtomScene( atom );
            }

            RpClumpDestroy( item );
        }

        RwImportedScan::Unapply();

        info->SetClump( clump );
        return true;
    }

    bool isVehicle = info->GetModelType() == MODEL_VEHICLE;
    
    if ( !RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
        return false;

    // MTA extension: include our imported textures
    RwImportedScan::Apply( info->m_textureDictionary );

    if ( isVehicle )
    {
        RpPrtStdGlobalDataSetStreamEmbedded( info );
        RwRemapScan::Apply();
    }

    RpClump *clump = RpClumpStreamRead( stream );

    if ( isVehicle )
    {
        RwRemapScan::Unapply();
        RpPrtStdGlobalDataSetStreamEmbedded( NULL );
    }

    RwImportedScan::Unapply();

    if ( !clump )
        return false;

    _initClumpScene( clump );

    info->SetClump( clump );

    // Game fix??? R*
    if ( id == VT_JOURNEY )
        ((CVehicleModelInfoSAInterface*)info)->m_numberOfDoors &= 0x02;

    return true;
}

/*=========================================================
    RwTexDictionaryLoadFirstHalf

    Arguments:
        stream - binary stream which contains the TXD
    Purpose:
        Loads half of the textures from a TexDictionary found in
        the provided stream. The rest of it is loaded in another
        function. This way execution time is (somewhat) flattened out.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00731070
=========================================================*/
static unsigned int *const VAR_NumTXDBlocks = (unsigned int*)0x008D6088;

static RwTexDictionary* RwTexDictionaryLoadFirstHalf( RwStream *stream )
{
    *VAR_NumTXDBlocks = 0;

    unsigned int version;
    unsigned int length;

    if ( !RwStreamFindChunk( stream, 1, &length, &version ) )
        return NULL;

    RwBlocksInfo info;

    if ( RwStreamReadBlocks( stream, info, length ) != length )
        return NULL;

    RwTexDictionary *txd = pGame->GetTextureManager()->RwCreateTexDictionary();

    if ( !txd )
        return NULL;

    unsigned short numBlocksHalf = info.count / 2;

    *VAR_NumTXDBlocks = numBlocksHalf;

    while ( info.count > numBlocksHalf )
    {
        RwTexture *tex = RwTextureStreamReadEx( stream );

        if ( !tex )
        {
            RwTexDictionaryDestroy( txd );
            return NULL;
        }

        tex->AddToDictionary( txd );

        info.count--;
    }

    *(unsigned int*)0x00C87FE8 = stream->data.position;
    return txd;
}

/*=========================================================
    LoadTXDFirstHalf

    Arguments:
        id - index of the TXD instance
        stream - binary stream which contains the TXD
    Purpose:
        Checks whether the stream contains a TXD. If not, it
        returns false. Then it returns whether the loading of
        half the TexDictionary into the TXD instance was
        successful.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00731930
=========================================================*/
static bool __cdecl LoadTXDFirstHalf( unsigned int id, RwStream *stream )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !RwStreamFindChunk( stream, 0x16, NULL, NULL ) )
        return false;

    return ( txd->m_txd = RwTexDictionaryLoadFirstHalf( stream ) ) != NULL;
}

/*=========================================================
    RegisterCOLLibraryModel

    Arguments:
        collId - index of the COL library
        modelId - model info id to which a collision was applied
    Purpose:
        Extends the range of applicability for the given COL lib.
        This loading of collisions for said COL library can be
        limited to a range so that future loading attempts are
        boosted.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00410820
=========================================================*/
void __cdecl RegisterCOLLibraryModel( unsigned short collId, unsigned short modelId )
{
    CColFileSA *col = (*ppColFilePool)->Get( collId );

    if ( (short)modelId < col->m_rangeStart )
        col->m_rangeStart = (short)modelId;

    if ( (short)modelId > col->m_rangeEnd )
        col->m_rangeEnd = (short)modelId;
}

// Update: added support for version 4 collision
bool __cdecl ReadCOLLibraryGeneral( const char *buf, size_t size, unsigned char collId )
{
    CBaseModelInfoSAInterface *info = NULL;

    while ( size > 8 )
    {
        const ColModelFileHeader& header = *(const ColModelFileHeader*)buf;

        buf += sizeof(header);

        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        unsigned short modelId = header.modelId;

        // Collisions may come with a cached model id.
        // Valid ids skip the need for name-checking.
        if ( modelId < DATA_TEXTURE_BLOCK )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->m_hash )
            info = CStreaming__GetModelByHash( hash, &modelId );

        // I am not a fan of uselessly big scopes.
        // The closer the code is to the left border, the easier it is to read for everybody.
        // Compilers do optimize goto.
        if ( !info )
            goto skip;

        // Update collision boundaries
        RegisterCOLLibraryModel( collId, modelId );

        // I do not expect collision replacements to be loaded this early.
        // The engine does preload the world collisions once. Further
        // loadings will be faster due to limitation of model scans to
        // id regions.
        assert( g_colReplacement[modelId] == NULL );

        if ( !info->IsDynamicCol() )
            goto skip;

        CColModelSAInterface *col = new CColModelSAInterface();

        switch( header.checksum )
        {
        case '4LOC':
            LoadCollisionModelVer4( buf, header.size - 0x18, col, header.name );
            break;
        case '3LOC':
            LoadCollisionModelVer3( buf, header.size - 0x18, col, header.name );
            break;
        case '2LOC':
            LoadCollisionModelVer2( buf, header.size - 0x18, col, header.name );
            break;
        default:
            LoadCollisionModel( buf, col, header.name );
            break;
        }

        // MTA extension: Put it into our global storage
        g_originalCollision[modelId] = col;

        col->m_colPoolIndex = collId;

        info->SetColModel( col, true );
        SetCachedCollision( modelId, col );

skip:
        size -= header.size;
        buf += header.size - (sizeof(ColModelFileHeader) - 8);
    }

    return true;
}

bool __cdecl ReadCOLLibraryBounds( const char *buf, size_t size, unsigned char collId )
{
    CBaseModelInfoSAInterface *info = NULL;

    while ( size > 8 )
    {
        const ColModelFileHeader& header = *(const ColModelFileHeader*)buf;

        buf += sizeof(header);

        // Note: this function has version 4 support by default!
        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        unsigned short modelId = header.modelId;

        if ( modelId < DATA_TEXTURE_BLOCK )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->m_hash )
        {
            CColFileSA *colFile = (*ppColFilePool)->Get( collId );

            info = CStreaming__GetModelInfoByName( header.name, (unsigned short)colFile->m_rangeStart, (unsigned short)colFile->m_rangeEnd, &modelId );
        }

        if ( info && info->IsDynamicCol() )
        {
            CColModelSAInterface *col;

            if ( CColModelSA *colInfo = g_colReplacement[modelId] )
            {
                // MTA extension: We store it in our data, so we can restore to it later
                col = colInfo->GetOriginal();

                if ( !col )
                    colInfo->SetOriginal( col = new CColModelSAInterface() );
            }
            else
            {
                // The original route
                col = info->m_pColModel;

                if ( !col )
                {
                    col = new CColModelSAInterface();

                    info->SetColModel( col, true );
                }
            }

            switch( header.checksum )
            {
            case '4LOC':
                LoadCollisionModelVer4( buf, header.size - 0x18, col, header.name );
                break;
            case '3LOC':
                LoadCollisionModelVer3( buf, header.size - 0x18, col, header.name );
                break;
            case '2LOC':
                LoadCollisionModelVer2( buf, header.size - 0x18, col, header.name );
                break;
            default:
                LoadCollisionModel( buf, col, header.name );
                break;
            }

            // MTA extension: Put it into our global storage
            g_originalCollision[modelId] = col;

            col->m_colPoolIndex = collId;

            // Do some procedural object logic
            if ( info->GetModelType() == MODEL_ATOMIC )
                ((bool (__cdecl*)( CBaseModelInfoSAInterface *info ))0x005DB650)( info );
        }
        
        size -= header.size;
        buf += header.size - (sizeof(ColModelFileHeader) - 8);
    }

    return true;
}

bool __cdecl LoadCOLLibrary( unsigned char collId, const char *buf, size_t size )
{
    CColFileSA *col = (*ppColFilePool)->Get( collId );
    bool success;

    // If the library was previously loaded and knows its regions, call ReadCOLLibaryBounds.
    // Otherwise we perform a global replacement (ReadCOLLibraryGeneral), to cache the id region.
    if ( col->m_rangeStart > col->m_rangeEnd )
        success = ReadCOLLibraryGeneral( buf, size, collId );
    else
        success = ReadCOLLibraryBounds( buf, size, collId );

    // Mark that the library is ready for interaction!
    if ( success )
        col->m_loaded = true;

    return success;
}

void __cdecl FreeCOLLibrary( unsigned char collId )
{
    CColFileSA *col = (*ppColFilePool)->Get( collId );

    // We kinda need another load to function.
    col->m_loaded = false;

    for ( short n = col->m_rangeStart; n <= col->m_rangeEnd; n++ )
    {
        // MTA extension: GTA:SA may not touch replaced collision data.
        if ( g_colReplacement[n] )
            continue;

        CBaseModelInfoSAInterface *info = ppModelInfo[n];

        if ( !info )
            continue;

        CColModelSAInterface *colModel = info->m_pColModel;

        if ( colModel && info->IsDynamicCol() && colModel->m_colPoolIndex == collId )
            colModel->ReleaseData();
    }
}

bool __cdecl LoadModel( void *buf, unsigned int id, unsigned int threadId )
{
    CModelLoadInfoSA& loadInfo = VAR_ModelLoadInfo[id];

    RwBuffer streamBuffer;
    streamBuffer.ptr = buf;
    streamBuffer.size = loadInfo.m_blockCount * 2048;

    RwStream *stream = RwStreamInitialize( (void*)0x008E48AC, 0, 3, 1, &streamBuffer );

    if ( id < DATA_TEXTURE_BLOCK )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[id];
        int animIndex = info->GetAnimFileIndex();

        CTxdInstanceSA *txdInst = (*ppTxdPool)->Get( info->m_textureDictionary );
        CAnimBlockSAInterface *animBlock;

        if ( txdInst->m_txd == NULL )
            goto failure;

        if ( animIndex != 0xFFFFFFFF )
        {
            animBlock = pGame->GetAnimManager()->GetAnimBlock( animIndex );

            if ( !animBlock->m_loaded )
                goto failure;
        }
        else
            animBlock = NULL;

        // Reference the resources
        txdInst->Reference();
        
        // ... and anim block
        if ( animBlock )
            animBlock->Reference();

        txdInst->SetCurrent();

        eRwType type = info->GetRwModelType();
        bool success;

        if ( type == RW_ATOMIC )
        {
            RtDict *dict;

            RwChunkHeader header;
            RwStreamReadChunkHeaderInfo( stream, header );

            // Check if there is animation attached
            if ( header.id == 0x2B )
                animDict->m_current = dict = RtDictSchemaStreamReadDict( animDict, stream );
            else
                dict = NULL;

            // At this point, GTA_SA utilizes a weird stream logic
            // I have fixed it here (bad clean-up of stream pointers)
            RwStreamClose( stream, &streamBuffer );
            stream = RwStreamInitialize( (void*)0x008E48AC, 0, 3, 1, &streamBuffer );

            success = LoadClumpFile( stream, id );

            if ( dict )
                RtDictDestroy( dict );
        }
        else
            success = LoadClumpFilePersistent( stream, id );

        // Replace collision if necessary
        if ( CColModelSA *col = g_colReplacement[id] )
        {
            CColModelSAInterface *icol = info->m_pColModel;
            CColModelSAInterface *ocol = col->GetOriginal();

            if ( icol )
            {
                if ( icol != ocol )
                    delete ocol;    // if its NULL, wont crash

                col->SetOriginal( icol );
            }
        }

        if ( loadInfo.m_eLoading != MODEL_RELOAD )
        {
            txdInst->DereferenceNoDestroy();

            if ( animBlock )
                animBlock->m_references--;

            if ( !success )
                goto failure;
          
            if ( info->GetModelType() == MODEL_VEHICLE )
                success = ((bool (__cdecl*)( unsigned int id ))0x00408000)( id );
        }

        if ( !success )
            goto failure;
    }
    else if ( id < DATA_TEXTURE_BLOCK + MAX_TXD )
    {
        unsigned short txdId = id - DATA_TEXTURE_BLOCK;
        CTxdInstanceSA *txdInst = (*ppTxdPool)->Get( txdId );

        if ( txdInst->m_parentTxd != 0xFFFF )
        {
            if ( !(*ppTxdPool)->Get( txdInst->m_parentTxd )->m_txd )
                goto failure;
        }

        // Check the threaded streaming environment (I guess?)
        if ( !( loadInfo.m_flags & 0x0E ) && !((bool (__cdecl*)( unsigned int txdID ) )0x00409A90)( txdId ) )
            goto failureDamned;

        bool successLoad;

        if ( *(bool*)0x008E4A58 )
        {
            successLoad = LoadTXDFirstHalf( txdId, stream );

            if ( !successLoad )
                goto failure;

            loadInfo.m_eLoading = MODEL_RELOAD;
        }
        else
        {
            successLoad = txdInst->LoadTXD( stream );

            if ( successLoad )
                txdInst->InitParent();
        }

        if ( !successLoad )
            goto failure;
    }
    else if ( id < 25255 )  // collision
    {
        if ( !LoadCOLLibrary( (unsigned char)( id - 25000 ), (const char*)buf, streamBuffer.size ) )
            goto failure;
    }
    else if ( id < 25511 )
    {
        if ( !((bool (__cdecl*)( unsigned int iplId, const void *data, size_t size ))0x00406080)( id - 25255, buf, streamBuffer.size ) )
            goto failure;
    }
    else if ( id < 25575 )
    {
        __asm
        {
            mov eax,id
            sub eax,25511
            push eax
            push stream
            mov ecx,CLASS_CPathFind
            mov eax,0x004529F0
            call eax
        }
    }
    else if ( id < 25755 )
    {
        if ( loadInfo.m_flags & 0x0E || ((bool (__cdecl*)( unsigned int id ))0x00407AD0)( id - 25575 ) )
        {
            pGame->GetAnimManager()->LoadAnimFile( stream, true, NULL );
            pGame->GetAnimManager()->CreateAnimAssocGroups();
        }
        else
            goto failureDamned;
    }
    else if ( id < 26230 )
    {
        ((void (__cdecl*)( RwStream *stream, unsigned int id, size_t size ))0x0045A8F0)( stream, id - 25755, streamBuffer.size );
    }
    else
    {
        // Skip loading scripts (are not utilized in MTA anyway)
        // If we need them in future, no problem to add.
        goto failure;
    }
    
    RwStreamClose( stream, &streamBuffer );

    if ( id < DATA_TEXTURE_BLOCK )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[id];
        eModelType type = info->GetModelType();

        if ( type != MODEL_VEHICLE && type != MODEL_PED )   // Well, there also is weapon model info?
        {
            if ( CAtomicModelInfoSA *atomInfo = info->GetAtomicModelInfo() )
                atomInfo->m_alpha = ( loadInfo.m_flags & 0x24 ) ? 0xFF : 0;

            if ( loadInfo.m_flags & 0x06 )
                goto finish;

            __asm
            {
                mov eax,ds:[0x008E4C60]
                push eax
                mov ecx,loadInfo
                mov eax,0x00407480
                call eax
            }
        }
    }
    else if ( id < 25000 || id >= 25575 && id < 25755 || id > 26230 )
    {
        if ( loadInfo.m_flags & 0x06 )
            goto finish;

        __asm
        {
            mov eax,ds:[0x008E4C60]
            push eax
            mov ecx,loadInfo
            mov eax,0x00407480
            call eax
        }
    }

finish:
    if ( loadInfo.m_eLoading != MODEL_RELOAD )
    {
        loadInfo.m_eLoading = MODEL_LOADED;
        (*(unsigned int*)0x008E4CB4) += streamBuffer.size;

        if ( streamingLoadCallback )
            streamingLoadCallback( id );
    }

    return true;

    // failure should be a commonly used routine in this function.
failure:
    CStreamingSA *streaming = pGame->GetStreaming();

    streaming->FreeModel( id );
    streaming->RequestModel( id, loadInfo.m_flags );

    RwStreamClose( stream, &streamBuffer );
    return false;

    // failureDamned is rarely used but important for reference/comparison here.
failureDamned:
    pGame->GetStreaming()->FreeModel( id );

    RwStreamClose( stream, &streamBuffer );
    return false;
}

void CStreamingSA::SetLoadCallback( streamingLoadCallback_t callback )
{
    streamingLoadCallback = callback;
}