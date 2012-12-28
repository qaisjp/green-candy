/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.utils.cpp
*  PURPOSE:     Data streamer utilities
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;
static RtDictSchema *const animDict =   (RtDictSchema*)0x008DED50;
static CModelLoadInfoSA *const VAR_ModelLoadInfo = (CModelLoadInfoSA*)0x008E4CC0;

static streamingLoadCallback_t  streamingLoadCallback = NULL;

static RwScanTexDictionaryStackRef_t    prevStackScan;

static RwTexture* RwTexDictionaryStackFindRemapRef( const char *name )
{
    RwTexture *tex = g_vehicleTxd->FindNamedTexture( name );

    if ( tex )
        return tex;

    // The_GTA: usually the engine flagged used remap textures with a '#'
    // We do not want this feature.
    return prevStackScan( name );
}

static void ApplyRemapTextureScan()
{
    prevStackScan = pRwInterface->m_textureManager.m_findInstanceRef;
    pRwInterface->m_textureManager.m_findInstanceRef = RwTexDictionaryStackFindRemapRef;
}

static void UnapplyRemapTextureScan()
{
    pRwInterface->m_textureManager.m_findInstanceRef = prevStackScan;
    prevStackScan = NULL;
}

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

static void _initAtomScene( RpAtomic *atom )
{
    atom->m_scene = *p_gtaScene;

    RpGeometry& geom = *atom->m_geometry;
    geom.flags |= RW_GEOMETRY_GLOBALLIGHT;

    // TODO: reenable this using multi-threading
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

static void RpClumpAtomicActivator( RpAtomic *atom, unsigned int replacerId )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[replacerId];
    CAtomicModelInfoSA *atomInfo = info->GetAtomicModelInfo();
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

// Dynamic Lighting fix
static void _initClumpScene( RpClump *clump )
{
    LIST_FOREACH_BEGIN( RpAtomic, clump->m_atomics.root, m_atomics )
        _initAtomScene( item );
    LIST_FOREACH_END
}

static bool __cdecl LoadClumpFile( RwStream *stream, unsigned int model )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[model];
    bool appliedRemapCheck, result;

    CAtomicModelInfoSA *atomInfo = info->GetAtomicModelInfo();

    if ( atomInfo && ( atomInfo->m_collFlags & COLL_WETROADREFLECT ) )
    {
        ApplyRemapTextureScan();
        appliedRemapCheck = true;
    }
    else
        appliedRemapCheck = false;

    result = false;

    if ( RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
    {
        RpClump *clump = RpClumpStreamRead( stream );

        if ( !clump )
            goto fail;

        while ( !LIST_EMPTY( clump->m_atomics.root ) )
            RpClumpAtomicActivator( LIST_GETITEM( RpAtomic, clump->m_atomics.root.next, m_atomics ), model );

        RpClumpDestroy( clump );

        result = atomInfo->m_rpAtomic != NULL;
    }

fail:
    if ( appliedRemapCheck )
        UnapplyRemapTextureScan();

    return result;
}

static bool __cdecl LoadClumpFilePersistent( RwStream *stream, unsigned int id )
{
    CClumpModelInfoSAInterface *info = (CClumpModelInfoSAInterface*)ppModelInfo[id];
    bool isVehicle = info->GetModelType() == MODEL_VEHICLE;
    RpClump *clump;
    RwFrame *frame;

    if ( info->m_renderFlags & RENDER_NOSKELETON )
    {
        clump = RpClumpCreate();
        clump->m_parent = frame = RwFrameCreate();

        while ( RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
        {
            RpClump *item = RpClumpStreamRead( stream );

            if ( !item )
            {
                // Small memory leak fix
                RwFrameDestroy( frame );
                clump->m_parent = NULL;

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

        info->SetClump( clump );
        return true;
    }
    
    if ( !RwStreamFindChunk( stream, 0x10, NULL, NULL ) )
        return false;

    if ( isVehicle )
    {
        RpPrtStdGlobalDataSetStreamEmbedded( info );
        ApplyRemapTextureScan();
    }

    clump = RpClumpStreamRead( stream );

    if ( isVehicle )
    {
        UnapplyRemapTextureScan();
        RpPrtStdGlobalDataSetStreamEmbedded( NULL );
    }

    if ( !clump )
        return false;

    _initClumpScene( clump );

    info->SetClump( clump );

    // Game fix???
    if ( id == VT_JOURNEY )
        ((CVehicleModelInfoSAInterface*)info)->m_numberOfDoors &= 0x02;

    return true;
}

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
        RwTexture *tex = RwStreamReadTexture( stream );

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

static bool __cdecl LoadTXDFirstHalf( unsigned int id, RwStream *stream )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !RwStreamFindChunk( stream, 0x16, NULL, NULL ) )
        return false;

    return ( txd->m_txd = RwTexDictionaryLoadFirstHalf( stream ) ) != NULL;
}

void __cdecl RegisterCOLLibraryModel( unsigned short collId, unsigned short modelId )
{
    CColFileSA *col = (*ppColFilePool)->Get( collId );

    if ( (short)modelId < col->m_upperBound )
        col->m_upperBound = (short)modelId;

    if ( (short)modelId > col->m_lowerBound )
        col->m_lowerBound = (short)modelId;
}

// Update: added support for version 4 collision
bool __cdecl ReadCOLLibraryGeneral( const char *buf, size_t size, unsigned char collId )
{
    CBaseModelInfoSAInterface *info = NULL;

    while ( size > 8 )
    {
        const ColModelFileHeader& header = *(const ColModelFileHeader*)buf;

        buf += sizeof(header);
        size -= sizeof(header);

        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        unsigned short modelId = header.modelId;

        if ( modelId < DATA_TEXTURE_BLOCK )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->m_hash )
            info = CStreaming__GetModelByHash( hash, &modelId );

        if ( !info )
            goto skip;

        // Update collision boundaries
        RegisterCOLLibraryModel( collId, modelId );

        bool isDynamic = IS_FLAG( info->m_renderFlags, RENDER_COLMODEL );
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

        // Put it into our global storage
        g_originalCollision[modelId] = col;

        col->m_colPoolIndex = collId;

        if ( isDynamic )
        {
            info->SetColModel( col, true );
            SetCachedCollision( modelId, col );
        }

skip:
        size += (0x18 - header.size);
        buf += header.size - 0x18;
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
        size -= sizeof(header);

        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        unsigned short modelId = header.modelId;

        if ( modelId < DATA_TEXTURE_BLOCK )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->m_hash )
        {
            CColFileSA *colFile = (*ppColFilePool)->Get( collId );

            info = CStreaming__GetModelInfoByName( header.name, (unsigned short)colFile->m_upperBound, (unsigned short)colFile->m_lowerBound, &modelId );
        }

        if ( info )
        {
            if ( !g_colReplacement[modelId] )
            {
                bool isDynamic = IS_FLAG( info->m_renderFlags, RENDER_COLMODEL );
                CColModelSAInterface *col = info->m_pColModel;

                if ( !col )
                {
                    col = new CColModelSAInterface();

                    if ( isDynamic )
                        info->SetColModel( col, true );
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

                // Put it into our global storage
                g_originalCollision[modelId] = col;

                col->m_colPoolIndex = collId;
            }

            // Do some procedural object logic
            if ( info->GetModelType() == MODEL_ATOMIC )
                ((bool (__cdecl*)( CBaseModelInfoSAInterface *info ))0x005DB650)( info );
        }
        
        size += (0x18 - header.size);
        buf += header.size - 0x18;
    }

    return true;
}

bool __cdecl LoadCOLLibrary( unsigned char collId, const char *buf, size_t size )
{
    CColFileSA *col = (*ppColFilePool)->Get( collId );
    bool success;

    if ( col->m_lowerBound <= col->m_upperBound )
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

    col->m_loaded = false;

    for ( unsigned short n = (unsigned short)col->m_upperBound; n < (unsigned short)col->m_lowerBound; n++ )
    {
        if ( g_colReplacement[n] )
            continue;

        CBaseModelInfoSAInterface *info = ppModelInfo[n];

        if ( !info )
            continue;

        CColModelSAInterface *col = info->m_pColModel;

        if ( col && info->IsDynamicCol() && col->m_colPoolIndex == collId )
            col->ReleaseData();
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
            // I have fixed it here
            RwStreamClose( stream, &streamBuffer );
            stream = RwStreamInitialize( (void*)0x008E48AC, 0, 3, 1, &streamBuffer );

            success = LoadClumpFile( stream, id );

            if ( dict )
                RtDictDestroy( dict );
        }
        else
            success = LoadClumpFilePersistent( stream, id );

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
            CTxdInstanceSA *txdInst = (*ppTxdPool)->Get( txdId );

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
        // Skip loading scripts
        goto failure;
    }
    
    RwStreamClose( stream, &streamBuffer );

    if ( id < DATA_TEXTURE_BLOCK )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[id];
        eModelType type = info->GetModelType();

        if ( type == MODEL_VEHICLE || type == MODEL_PED )
            goto finish;

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

failure:
    CStreamingSA *streaming = pGame->GetStreaming();

    streaming->FreeModel( id );
    streaming->RequestModel( id, loadInfo.m_flags );

    RwStreamClose( stream, &streamBuffer );
    return false;

failureDamned:
    pGame->GetStreaming()->FreeModel( id );

    RwStreamClose( stream, &streamBuffer );
    return false;
}

void CStreamingSA::SetLoadCallback( streamingLoadCallback_t callback )
{
    streamingLoadCallback = callback;
}