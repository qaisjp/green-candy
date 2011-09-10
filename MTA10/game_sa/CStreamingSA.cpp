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

#define DATA_TEXTURE_BLOCK      20000
#define DATA_ANIM_BLOCK         25575

#define ARRAY_PEDSPECMODEL      0x008E4C00
#define VAR_PEDSPECMODEL        0x008E4BB0
#define VAR_SPECPTR             0x008E4CB4
#define VAR_NUMMODELS           0x008E4CB8
#define ARRAY_pLODModelLoaded   0x009654B4

#define ARRAY_MODELIDS          0x008E4A60
#define ARRAY_LODMODELIDS       0x008E4AF8

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
            (*(DWORD*)0x008E4BA0)++;

            info->m_flags |= 0x10;
        }
    }
    else if ( info->m_eLoading == MODEL_UNAVAILABLE )
    {
        flags &= ~0x10;
    }

    info->m_flags |= flags;

    // Refresh the model loading?
    if ( info->m_eLoading == MODEL_LOADED )
    {
        CModelLoadInfoSA *lodInfo;

        if ( info->m_lodModelID == 0xFFFF )
            return;

        lodInfo = (CModelLoadInfoSA*)*(DWORD*)ARRAY_pLODModelLoaded + info->m_lodModelID;
        lodInfo->m_unknown4 = info->m_unknown4;
        lodInfo->m_lodModelID = info->m_lodModelID;

        info->m_unknown4 = 0xFFFF;
        info->m_unknown4 = 0xFFFF;

        if ( id < DATA_TEXTURE_BLOCK )
        {
            CBaseModelInfoSAInterface *model = ppModelInfo[id];

            dwFunc = 0x00407480;

            switch( model->GetModelType() )
            {
            case MODEL_VEHICLE:
            case MODEL_PED:
                return;
            }

            if ( info->m_flags & (0x02 | 0x04) )
                return;

            __asm
            {
                mov eax,dword ptr [0x008E4C60]
                push eax
                mov eax,info
                call dwFunc
            }
        }

        return;
    }

    // Make sure we are really unloaded?
    switch( info->m_eLoading )
    {
    case MODEL_LOADING:
    case MODEL_LOD:
    case MODEL_UNKNOWN:
        return;
    case MODEL_LOADED:
        // If available, we reload the model
        info->m_flags = flags;

        info->m_eLoading = MODEL_LOADING;
        return;
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
    else if ( id < 25000 )
    {
        int textureDictionary;
        dwFunc = 0x00408370;

        __asm
        {
            movsx eax,id
            sub eax,20000
            push eax
            call dwFunc
            pop textureDictionary
        }

        // I think it loads textures, lol
        if ( textureDictionary != -1 )
            RequestModel( textureDictionary + DATA_TEXTURE_BLOCK, flags );
    }

    dwFunc = 0x00407480;

    __asm
    {
        push dword ptr[0x008E4C58]
        mov ecx,info
        call dwFunc
    }

    (*(DWORD*)VAR_NUMMODELS)++;

    if ( flags & 0x10 )
        (*(DWORD*)0x008E4BA0)++;
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

    if (info->m_eLoading == MODEL_UNAVAILABLE)
        return;

    if (id < DATA_TEXTURE_BLOCK)
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

            while (unk2 < (unsigned int*)ARRAY_PEDSPECMODEL + 5)
            {
                if (*unk2 == id)
                {
                    *unk2 = 0xFFFFFFFF;

                    unk--;
                }
            }

            *(int*)VAR_PEDSPECMODEL = unk;

            break;
        case MODEL_VEHICLE:
            dwFunc = 0x004080F0;

            __asm
            {
                movsx eax,id
                push eax
                call dwFunc
                pop eax
            }

            break;
        }
    }
    else if ( id < 25000 )
    {
        // Remove texture reference?
        CTxdStore_RemoveTxd( id - 20000 );
    }
    else if ( id < 25255 )
    {
        dwFunc = 0x00410730;

        // Something to do with ped models?
        __asm
        {
            movsx eax,id
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
            movsx eax,id
            sub eax,25255

            push eax
            call dwFunc
            pop eax
        }
    }
    else if ( id < DATA_ANIM_BLOCK )
    {
        DWORD dwFunc = 0x0044D0F0;

        __asm
        {
            movsx eax,id
            sub eax,25511

            push eax
            call dwFunc
            pop eax
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
            movsx eax,id
            sub eax,26230

            mov ecx,0x00A47B60
            push eax
            call dwFunc
        }
    }

    *(DWORD*)VAR_SPECPTR -= info->m_unknown3 << 11;

    if ( info->m_lodModelID != 0xFFFFFFFF )
    {
        CModelLoadInfoSA *lodInfo = (CModelLoadInfoSA*)(*(DWORD*)ARRAY_pLODModelLoaded) + info->m_lodModelID;

        if ( info->m_eLoading == MODEL_LOADING )
        {
            (*(DWORD*)VAR_NUMMODELS)--;

            if ( info->m_flags & 0x10 )
            {
                info->m_flags &= ~0x10;

                (*(DWORD*)0x008E4BA0)--;
            }
        }

        lodInfo->m_unknown4 = info->m_unknown4;
        lodInfo->m_lodModelID = info->m_lodModelID;

        info->m_unknown4 = 0xFFFF;
        info->m_lodModelID = 0xFFFF;
    }
    else if ( info->m_eLoading == MODEL_LOD )
    {
        unsigned int n;

        for (n=0; n<30; n++)
        {
            if (*((int*)ARRAY_MODELIDS + n) == id)
                *((int*)ARRAY_MODELIDS + n) = -1;

            if (*((int*)ARRAY_LODMODELIDS + n) == id)
                *((int*)ARRAY_LODMODELIDS + n) = -1;
        }
    }
    else if ( info->m_eLoading == MODEL_UNKNOWN )
    {
        // What the... Its removal all over again, I think this is a bug.
        // 00408B98

        assert ( 0 );
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