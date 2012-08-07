/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.cpp
*  PURPOSE:     Data streamer
*  DEVELOPERS:  Jax <>
*               jenksta <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

#define ARRAY_PEDSPECMODEL      0x008E4C00
#define VAR_PEDSPECMODEL        0x008E4BB0
#define VAR_MEMORYUSAGE         0x008E4CB4
#define VAR_NUMMODELS           0x008E4CB8
#define VAR_NUMPRIOMODELS       0x008E4BA0
#define ARRAY_pLODModelLoaded   0x009654B4

#define ARRAY_MODELIDS          0x008E4A60
#define ARRAY_LODMODELIDS       0x008E4AF8

#define FLAG_PRIORITY           0x10

static void __cdecl HOOK_CStreaming__RequestModel( unsigned int model, unsigned int flags )
{
    pGame->GetStreaming()->RequestModel( model, flags );
}

static void __cdecl HOOK_CStreaming__FreeModel( unsigned int model )
{
    pGame->GetStreaming()->FreeModel( model );
}

CStreamingSA::CStreamingSA()
{
    // Hook the model requested
    HookInstall( FUNC_CStreaming__RequestModel, (DWORD)HOOK_CStreaming__RequestModel, 6 );
    HookInstall( 0x004089A0, (DWORD)HOOK_CStreaming__FreeModel, 6 );
}

CStreamingSA::~CStreamingSA()
{
}

void CStreamingSA::RequestModel( unsigned short id, unsigned int flags )
{
    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;
    DWORD dwFunc;

    if ( id > MAX_MODELS-1 )
        return;

    if ( info->m_eLoading == MODEL_LOADING )
    {
        if ( flags & 0x10 && !(info->m_flags & 0x10) )
        {
            (*(DWORD*)VAR_NUMPRIOMODELS)++;

            info->m_flags |= FLAG_PRIORITY;
        }
    }
    else if ( info->m_eLoading != MODEL_UNAVAILABLE )
    {
        flags &= ~FLAG_PRIORITY;
    }

    info->m_flags |= (unsigned char)flags;

    // Refresh the model loading?
    if ( info->m_eLoading == MODEL_LOADED )
    {
        CModelLoadInfoSA *lodInfo;

        if ( info->m_lodModelID == 0xFFFF )
            return;

        // Unfold loaded model
        lodInfo = (CModelLoadInfoSA*)*(DWORD*)ARRAY_pLODModelLoaded + info->m_lodModelID;
        lodInfo->m_unknown4 = info->m_unknown4;
        lodInfo = (CModelLoadInfoSA*)*(DWORD*)ARRAY_pLODModelLoaded + info->m_unknown4;
        lodInfo->m_lodModelID = info->m_lodModelID;

        info->m_unknown4 = 0xFFFF;
        info->m_lodModelID = 0xFFFF;

        if ( id < DATA_TEXTURE_BLOCK )
        {
            CBaseModelInfoSAInterface *model = ppModelInfo[id];

            switch( model->GetModelType() )
            {
            case MODEL_VEHICLE:
            case MODEL_PED:
                return;
            }
        }

        if ( info->m_flags & (0x02 | 0x04) )
            return;

        dwFunc = 0x00407480;

        __asm
        {
            mov eax,ds:[0x008E4C60]
            push eax
            mov ecx,info
            call dwFunc
        }

        return;
    }

    // Make sure we are really unloaded?
    switch( info->m_eLoading )
    {
    case MODEL_LOADING:
    case MODEL_LOD:
    case MODEL_RELOAD:
        return;
    case MODEL_LOADED:
        goto reload;
    }

    if ( id < DATA_TEXTURE_BLOCK )
    {
        CBaseModelInfoSAInterface *model = ppModelInfo[id];
        int animIndex = model->GetAnimFileIndex();

        // Accelerate our textures, yay
        RequestModel( model->m_textureDictionary + DATA_TEXTURE_BLOCK, flags );

        // Get textures if necessary
        if ( animIndex != -1 )
            RequestModel( animIndex + DATA_ANIM_BLOCK, 0x08 );
    }
    else if ( id < DATA_TEXTURE_BLOCK + MAX_TXD )
    {
        CTxdInstanceSA *txd = (*ppTxdPool)->Get( id - DATA_TEXTURE_BLOCK );

        // Crashfix
        if ( !txd )
            return;

#ifdef MTA_DEBUG
        OutputDebugString( SString( "loaded texDictionary %u\n", id - DATA_TEXTURE_BLOCK ) );
#endif

        // I think it loads textures, lol
        if ( txd->m_parentTxd != 0xFFFF )
            RequestModel( txd->m_parentTxd + DATA_TEXTURE_BLOCK, flags );
    }

    dwFunc = 0x00407480;

    __asm
    {
        mov eax,ds:[0x008E4C58]
        push eax
        mov ecx,info
        call dwFunc
    }

    (*(DWORD*)VAR_NUMMODELS)++;

    if ( flags & 0x10 )
        (*(DWORD*)VAR_NUMPRIOMODELS)++;

reload:
    // If available, we reload the model
    info->m_flags = flags;

    info->m_eLoading = MODEL_LOADING;
}

void CStreamingSA::FreeModel( unsigned short id )
{
    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;
    CBaseModelInfoSAInterface *model;
    DWORD dwFunc;

    if ( id > MAX_MODELS-1 )
        return;

    if ( info->m_eLoading == MODEL_UNAVAILABLE )
        return;

    if ( info->m_eLoading == MODEL_LOADED )
    {
        if ( id < DATA_TEXTURE_BLOCK )
        {
            int unk;
            unsigned int *unk2;

            model = ppModelInfo[id];

            model->DeleteRwObject();

            switch( model->GetModelType() )
            {
            case MODEL_PED:
                unk = *(int*)VAR_PEDSPECMODEL;
                unk2 = (unsigned int*)ARRAY_PEDSPECMODEL;

                while ( unk2 < (unsigned int*)ARRAY_PEDSPECMODEL + 5 )
                {
                    if (*unk2 == id)
                    {
                        *unk2 = 0xFFFFFFFF;

                        unk--;
                    }

                    unk2++;
                }

                *(int*)VAR_PEDSPECMODEL = unk;

                break;
            case MODEL_VEHICLE:
#ifdef MTA_DEBUG
                OutputDebugString( SString( "deleted vehicle model %u\n", id ) );
#endif

                ((void (__cdecl*)( unsigned int ))0x004080F0)( id );

                break;
            }
        }
        else if ( id < DATA_TEXTURE_BLOCK + MAX_TXD )
        {
#ifdef MTA_DEBUG
            OutputDebugString( SString( "Deleted texDictionary %u\n", id - DATA_TEXTURE_BLOCK ) );
#endif

            // Remove texture reference?
            (*ppTxdPool)->Get( id - DATA_TEXTURE_BLOCK )->Deallocate();
        }
        else if ( id < 25255 )
        {
            dwFunc = 0x00410730;

            // Something to do with ped models?
            __asm
            {
                movzx eax,id
                sub eax,25000

                push eax
                call dwFunc
                pop eax
            }
        }
        else if ( id < 25511 )
        {
            dwFunc = 0x00404B20;

            __asm
            {
                movzx eax,id
                sub eax,25255

                push eax
                call dwFunc
                pop eax
            }
        }
        else if ( id < DATA_ANIM_BLOCK )    // path finding...?
        {
            DWORD dwFunc = 0x0044D0F0;

            __asm
            {
                movzx eax,id
                sub eax,25511

                push eax
                mov ecx,CLASS_CPathFind
                call dwFunc
            }
        }
        else if ( id < 25755 )
        {
            // Animations...?
            pGame->GetAnimManager()->RemoveAnimBlock( id - DATA_ANIM_BLOCK );
        }
        else if ( id >= 26230 )
        {
            dwFunc = 0x004708E0;

            __asm
            {
                movzx eax,id
                sub eax,26230

                mov ecx,0x00A47B60
                push eax
                call dwFunc
            }
        }

        *(DWORD*)VAR_MEMORYUSAGE -= info->m_blockCount * 2048;
    }

    if ( info->m_lodModelID != 0xFFFF )
    {
        if ( info->m_eLoading == MODEL_LOADING )
        {
            (*(DWORD*)VAR_NUMMODELS)--;

            if ( info->m_flags & FLAG_PRIORITY )
            {
                info->m_flags &= ~FLAG_PRIORITY;

                (*(DWORD*)VAR_NUMPRIOMODELS)--;
            }
        }

        CModelLoadInfoSA *lodInfo = (CModelLoadInfoSA*)(*(DWORD*)ARRAY_pLODModelLoaded) + info->m_lodModelID;
        lodInfo->m_unknown4 = info->m_unknown4;

        lodInfo = (CModelLoadInfoSA*)(*(DWORD*)ARRAY_pLODModelLoaded) + info->m_unknown4;
        lodInfo->m_lodModelID = info->m_lodModelID;

        info->m_unknown4 = 0xFFFF;
        info->m_lodModelID = 0xFFFF;
    }
    else if ( info->m_eLoading == MODEL_LOD )
    {
        unsigned int n;

        for ( n=0; n<30; n++ )
        {
            if (*((int*)ARRAY_MODELIDS + n) == (int)id)
                *((int*)ARRAY_MODELIDS + n) = -1;

            if (*((int*)ARRAY_LODMODELIDS + n) == (int)id)
                *((int*)ARRAY_LODMODELIDS + n) = -1;
        }
    }
    else if ( info->m_eLoading == MODEL_RELOAD )
    {
        if ( id < DATA_TEXTURE_BLOCK )
            RwFlushLoader();
        else if ( id < 25000 )
            (*ppTxdPool)->Get( id - DATA_TEXTURE_BLOCK )->Deallocate();
        else if ( id < 25255 )
            ( (void (*)( unsigned int model ))0x00410730 )( id - 25000 );
        else if ( id < 25511 )
            ( (void (*)( unsigned int model ))0x00404B20 )( id - 25255 );
        else if ( id >= DATA_ANIM_BLOCK && id < 25755 )
            pGame->GetAnimManager()->RemoveAnimBlock( id - DATA_ANIM_BLOCK );
        else if ( id >= 26230 )
            ( (void (__stdcall*)( unsigned int model ))0x004708E0 )( id - 26230 );
    }

    info->m_eLoading = MODEL_UNAVAILABLE;
}

void CStreamingSA::LoadAllRequestedModels ( BOOL bOnlyPriorityModels )
{
    DWORD dwFunction = FUNC_LoadAllRequestedModels;
    DWORD dwOnlyPriorityModels = bOnlyPriorityModels;
    _asm
    {
        push    dwOnlyPriorityModels
        call    dwFunction
        add     esp, 4
    }
}

bool CStreamingSA::HasModelLoaded ( unsigned int id )
{
    DWORD dwFunc = FUNC_CStreaming__HasModelLoaded;
    bool bReturn;

    _asm
    {
        push    id
        call    dwFunc
        movzx   eax, al
        mov     bReturn, al
        pop     eax
    }

    return bReturn;
}

bool CStreamingSA::IsModelLoading ( unsigned int id )
{
    return ((CModelLoadInfoSA*)(ARRAY_CModelLoadInfo) + id)->m_eLoading == MODEL_LOADING;
}

void CStreamingSA::WaitForModel ( unsigned int id )
{
    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;

    if (id > MAX_MODELS-1)
        return;
}

void CStreamingSA::RequestAnimations ( int iAnimationLibraryBlock, DWORD dwFlags )
{
    iAnimationLibraryBlock += DATA_ANIM_BLOCK;
    RequestModel( iAnimationLibraryBlock, dwFlags );
}

BOOL CStreamingSA::HaveAnimationsLoaded ( int iAnimationLibraryBlock )
{
    iAnimationLibraryBlock += DATA_ANIM_BLOCK;
    return HasModelLoaded( iAnimationLibraryBlock );
}

bool CStreamingSA::HasVehicleUpgradeLoaded ( int model )
{
    bool bReturn;
    DWORD dwFunc = FUNC_CStreaming__HasVehicleUpgradeLoaded;
    _asm
    {
        push    model
        call    dwFunc
        add     esp, 0x4
        mov     bReturn, al
    }
    return bReturn;
}

void CStreamingSA::RequestSpecialModel ( DWORD model, const char * szTexture, DWORD channel )
{
    DWORD dwFunc = FUNC_CStreaming_RequestSpecialModel;
    _asm
    {
        push    channel
        push    szTexture
        push    model
        call    dwFunc
        add     esp, 0xC
    }
}