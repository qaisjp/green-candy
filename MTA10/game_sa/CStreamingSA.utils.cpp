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

static bool RpClumpAtomicActivator( RpAtomic *atom, unsigned int replacerId )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[replacerId];
    CAtomicModelInfoSA *atomInfo = info->GetAtomicModelInfo();
    bool unk;
    char unk2[24];

    // This possibly adds the reference to the texture, we should reven this
    ((void (__cdecl*)(const char*, char*, bool&))0x005370A0)( atom->m_parent->m_nodeName, unk2, unk );

    atom->SetRenderCallback( NULL );

    if ( unk )
        atomInfo->GetDamageAtomicModelInfo()->SetupPipeline( atom );
    else
        atomInfo->SetAtomic( atom );

    atom->RemoveFromClump();

    atom->AddToFrame( RwFrameCreate() );

    // Dyn Lighting fix: Apply the scene
    atom->m_scene = *p_gtaScene;
    atom->m_geometry->flags |= 0x20;
    
    atom->SetExtendedRenderFlags( replacerId );
    return true;
}

// Dynamic Lighting fix
static void _initClumpScene( RpClump *clump )
{
    LIST_FOREACH_BEGIN( RpAtomic, clump->m_atomics.root, m_atomics )
        item->m_scene = *p_gtaScene;
        item->m_geometry->flags |= 0x20;
    LIST_FOREACH_END

    LIST_FOREACH_BEGIN( RpLight, clump->m_lights.root, m_clumpLights )
        item->AddToScene( *p_gtaScene );
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

        clump->ForAllAtomics( RpClumpAtomicActivator, model );

        RpClumpDestroy( clump );

        result = atomInfo->m_rpAtomic != NULL;
    }

fail:
    if ( appliedRemapCheck )
        UnapplyRemapTextureScan();

    return result;
}

static bool RpClumpAtomicRegisterWithOwner( RpAtomic *_atom, RpClump *owner )
{
    RpAtomic *atom = RpAtomicClone( _atom );

    atom->AddToFrame( _atom->m_parent->m_root );
    atom->AddToClump( owner );
    return true;
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
            item->ForAllAtomics( RpClumpAtomicRegisterWithOwner, clump );

            RpClumpDestroy( item );
        }

        _initClumpScene( clump );

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

            RwStreamClose( stream, &streamBuffer );
            stream = RwStreamInitialize( (void*)0x008E48AC, 0, 3, 1, &streamBuffer );

            success = LoadClumpFile( stream, id );  // Is this actually a bug in the engine...?

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
        if ( !((bool (__cdecl*)( unsigned int collId, const void *data, size_t size ))0x004106D0)( id - 25000, buf, streamBuffer.size ) )
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