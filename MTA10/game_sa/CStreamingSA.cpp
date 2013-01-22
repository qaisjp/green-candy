/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
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

CIPLFilePool **ppIPLFilePool = (CIPLFilePool**)CLASS_CIPLFilePool;

extern CBaseModelInfoSAInterface **ppModelInfo;

CColModelSA *g_colReplacement[DATA_TEXTURE_BLOCK];
CColModelSAInterface *g_originalCollision[DATA_TEXTURE_BLOCK];

#define ARRAY_PEDSPECMODEL      0x008E4C00
#define VAR_PEDSPECMODEL        0x008E4BB0
#define VAR_MEMORYUSAGE         0x008E4CB4
#define VAR_NUMMODELS           0x008E4CB8
#define VAR_NUMPRIOMODELS       0x008E4BA0
#define ARRAY_pLODModelLoaded   0x009654B4

#define ARRAY_MODELIDS          0x008E4A60
#define ARRAY_LODMODELIDS       0x008E4AF8

#define FLAG_PRIORITY           0x10

static streamingRequestCallback_t streamingRequestCallback = NULL;
static streamingFreeCallback_t streamingFreeCallback = NULL;

static void __cdecl HOOK_CStreaming__RequestModel( unsigned int model, unsigned int flags )
{
    pGame->GetStreaming()->RequestModel( model, flags );
}

static void __cdecl HOOK_CStreaming__FreeModel( unsigned int model )
{
    pGame->GetStreaming()->FreeModel( model );
}

class CMissingModelInfoSA : public CSimpleItemStack <char[32]>
{
public:
    CMissingModelInfoSA( unsigned int max ) : CSimpleItemStack( max )
    {
    }

    inline void Add( const char *name )
    {
        char *alloc = *Allocate();

        if ( !alloc )
        {
            OutputDebugString( "Too many objects without modelinfo structures\n" );
            return;
        }

        memcpy( alloc, name, 32 );
    }
};

static HANDLE *const VAR_UnkFileHandle = (HANDLE*)0x008E4010;
static size_t *const VAR_NumResourceEntries = (size_t*)0x008E4C68;
static unsigned short *const VAR_BiggestPrimaryBlockOffset = (unsigned short*)0x008E4CA8;
static unsigned int *const VAR_LastModelScanIndex = (unsigned int*)0x00AAE948;
static CModelLoadInfoSA *const VAR_ModelLoadInfo = (CModelLoadInfoSA*)0x008E4CC0;
static CMissingModelInfoSA *const *VAR_MissingModelInfo = (CMissingModelInfoSA**)0x008E48D0;

CBaseModelInfoSAInterface* __cdecl CStreaming__GetModelByHash( unsigned int hash, unsigned short *id )
{
    unsigned int n = *VAR_LastModelScanIndex;
    CBaseModelInfoSAInterface *model;

    for ( ; n < DATA_TEXTURE_BLOCK; n++ )
    {
        model = ppModelInfo[n];

        if ( model && model->m_hash == hash )
            goto success;
    }

    n = *VAR_LastModelScanIndex;

    for ( ; n < DATA_TEXTURE_BLOCK; n-- )
    {
        model = ppModelInfo[n];

        if ( model && model->m_hash == hash )
            goto success;
    }

    return NULL;

success:
    if ( id )
        *id = (unsigned short)n;

    *VAR_LastModelScanIndex = n;
    return model;
}

CBaseModelInfoSAInterface* CStreaming__GetModelInfoByName( const char *name, unsigned short startId, unsigned short endId, unsigned short *id )
{
    unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( name );

    for ( unsigned short n = startId; n <= endId; n++ )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[n];

        if ( info && info->m_hash == hash )
        {
            if ( id )
                *id = n;

            return info;
        }
    }

    return NULL;
}

static void __cdecl TxdAssignVehiclePaintjob( const char *name, unsigned int id )
{
    unsigned char len = (unsigned char)strlen( name ) - 1;
    unsigned char lastchr = name[len];

    if ( !isdigit( name[len] ) )
        return;

    // Filter out the numbers
    char buf[24];

    // Bugfix: if a name consists of numbers entirely, we reject it
    for (;;)
    {
        if ( !( --len ) )
            return;

        if ( !isdigit( name[len] ) )
            break;
    }

    // Store the result
    strncpy( buf, name, ++len );
    buf[len] = '\0';

    CBaseModelInfoSAInterface *info = CStreaming__GetModelInfoByName( buf, 400, 630, NULL );

    if ( !info || info->GetModelType() != MODEL_VEHICLE )
        return;

    // Put the paintjob into our modelinfo
    ((CVehicleModelInfoSAInterface*)info)->AssignPaintjob( id );
}

/*
    SECUmadness chapter 2: CStreaming__RegisterCollision

    While the previous secuROM encounter with OpenImgFile had been a mad ride, this function
    appears to have less protective measures. The annoying thing about reading it is the constant
    anti-trace methods considering that this function is nowhere worthy of such security.

    Apparrently this is one of the many routines which add complexity to other code-areas by referencing
    chunks of bytes as dword-values, while in reality there dword-value are parts of routines themselves!
    Luckily all pointers are within secuROM borders.

    But I have learned from my jonders: Spectating two versions of GTA:SA at a time, utilizing different
    obfuscation methods, is very helpful..!
*/

static unsigned int __cdecl CStreaming__RegisterCollision( const char *name )
{
    CColFileSA *col = new CColFileSA;

    if ( stricmp( name, "procobj" ) == 0 || stricmp( name, "proc_int" ) == 0 || stricmp( name, "proc_int2" ) == 0 )
        col->m_isProcedural = true;

    if ( strnicmp( name, "int_la", 6 ) == 0 ||
         strnicmp( name, "int_sf", 6 ) == 0 ||
         strnicmp( name, "int_veg", 7 ) == 0 ||
         strnicmp( name, "int_cont", 8 ) == 0 ||
         strnicmp( name, "gen_int1", 8 ) == 0 ||
         strnicmp( name, "gen_int2", 8 ) == 0 ||
         strnicmp( name, "gen_int3", 8 ) == 0 ||
         strnicmp( name, "gen_int4", 8 ) == 0 ||
         strnicmp( name, "gen_int5", 8 ) == 0 ||
         strnicmp( name, "gen_intb", 8 ) == 0 ||
         strnicmp( name, "savehous", 8 ) == 0 ||
         stricmp( name, "props" ) == 0 ||
         stricmp( name, "props2" ) == 0 ||   // Okay, I am unsure whether I caught all of the namechecking due to secuROM obfuscation
                                                // If there is a filename missing, feel free to append it here!
         strnicmp( name, "levelmap", 8 ) == 0 ||
         strnicmp( name, "stadint", 7 ) == 0 )
        col->m_isInterior = true;

    return (*ppColFilePool)->GetIndex( col );
}

void* CIPLFileSA::operator new ( size_t )
{
    return (*ppIPLFilePool)->Allocate();
}

void CIPLFileSA::operator delete ( void *ptr )
{
    (*ppIPLFilePool)->Free( (CIPLFileSA*)ptr );
}

static unsigned int __cdecl CStreaming__FindIPLFile( const char *name )
{
    unsigned int n;

    for ( n=0; n<MAX_IPL; n++ )
    {
        CIPLFileSA *ipl = (*ppIPLFilePool)->Get( n );

        if ( ipl && stricmp( ipl->m_name, name ) == 0 )
            return n;
    }

    return 0xFFFFFFFF;
}

static unsigned int __cdecl CStreaming__RegisterIPLFile( const char *name )
{
    CIPLFileSA *ipl = new CIPLFileSA;

    strcpy( ipl->m_name, name );

    return (*ppIPLFilePool)->GetIndex( ipl );
}

struct fileHeader
{
    size_t              offset;                         // 0
    unsigned short      primaryBlockOffset;             // 4
    unsigned short      secondaryBlockOffset;           // 6
    char                name[24];                       // 8
};

static void __cdecl CStreaming__LoadArchive( IMGFile& archive, unsigned int imgID )
{
    CFile *file = OpenGlobalStream( archive.name, "rb" );

    if ( !file )
        return;

    unsigned short lastID = 0xFFFF;
    union
    {
        char version[4];    // has to be "VER2"
        unsigned int checksum;
    };
    unsigned int numFiles;

    file->Read( version, 1, 4 );

    assert( checksum == '2REV' );
        
    numFiles = file->ReadInt();

    while ( numFiles-- )
    {
        fileHeader header;

        file->Read( &header, 1, sizeof(header) );

        if ( *VAR_BiggestPrimaryBlockOffset < header.primaryBlockOffset )
            *VAR_BiggestPrimaryBlockOffset = header.primaryBlockOffset;

        // Zero terminated for safety
        header.name[ sizeof(header.name) - 1 ] = '\0';

        char *dot = strchr( header.name, '.' );

        if ( !dot )
        {
            lastID = 0xFFFF;
            continue;
        }

        const char *ext = dot + 1;

        if ( (size_t)( ext - header.name ) > 20 )
        {
            lastID = 0xFFFF;
            continue;
        }

        *dot = '\0';

        unsigned short id;

        if ( strnicmp( ext, "DFF", 3 ) == 0 )
        {
            if ( CStreaming__GetModelByHash( pGame->GetKeyGen()->GetUppercaseKey( header.name ), &id ) )
            {
                header.offset |= imgID << 24;

                // Some sort of debug container
                (*VAR_MissingModelInfo)->Add( header.name );
                
                lastID = 0xFFFF;
                continue;
            }
        }
        else if ( strnicmp( ext, "TXD", 3 ) == 0 )
        {
            id = pGame->GetTextureManager()->FindTxdEntry( header.name );

            if ( id == 0xFFFF )
            {
                id = pGame->GetTextureManager()->CreateTxdEntry( header.name );

                // Assign the txd to a vehicle if found a valid one
                TxdAssignVehiclePaintjob( header.name, id );
            }

            id += DATA_TEXTURE_BLOCK;
        }
        else if ( strnicmp( ext, "COL", 3 ) == 0 )
        {
            id = 25000 + CStreaming__RegisterCollision( header.name );
        }
        else if ( strnicmp( ext, "IPL", 3 ) == 0 )
        {
            id = CStreaming__FindIPLFile( header.name );

            if ( id == 0xFFFF )
            {
                id = CStreaming__RegisterIPLFile( header.name );
            }

            id += 25255;
        }
        else if ( strnicmp( ext, "DAT", 3 ) == 0 )
        {
            sscanf( header.name + 5, "%d", &id );
            
            id += 25511;
        }
        else if ( strnicmp( ext, "IFP", 3 ) == 0 )
        {
            id = 25575 + pGame->GetAnimManager()->RegisterAnimBlock( header.name );
        }
        else if ( strnicmp( ext, "RRR", 3 ) == 0 )
        {
            id = 25755 + pGame->GetRecordings()->Register( header.name );
        }
        else if ( strnicmp( ext, "SCM", 3 ) == 0 )
        {
            OutputDebugString( "found unsupported SCM file: " );
            OutputDebugString( header.name );
            OutputDebugString( "\n" );
            continue;
        }
        else
        {
            *dot = '.';
            lastID = 0xFFFF;
            continue;
        }

        unsigned int offset, count;

        if ( VAR_ModelLoadInfo[id].GetOffset( offset, count ) )
            continue;

        CModelLoadInfoSA& info = VAR_ModelLoadInfo[id];
        info.m_imgID = imgID;

        if ( header.secondaryBlockOffset != 0 )
            header.primaryBlockOffset = header.secondaryBlockOffset;

        info.SetOffset( header.offset, header.primaryBlockOffset );
        info.m_flags = 0;

        if ( lastID != 0xFFFF )
            VAR_ModelLoadInfo[lastID].m_lastID = id;

        lastID = id;
    }

    delete file;
}

static void __cdecl HOOK_CStreaming__LoadArchives()
{
    *(unsigned int*)0x008E4C90 = 0xFFFFFFFF;
    *(unsigned int*)0x008E4C94 = 0xFFFFFFFF;
    *(unsigned int*)0x008E4C98 = 0xFFFFFFFF;
    *(unsigned int*)0x008E4C9C = 0xFFFFFFFF;

    *(unsigned int*)0x008E4C8C = 0;

    *(unsigned int*)0x008E4CA0 = 0xFFFFFFFF;

    *VAR_NumResourceEntries = GetFileSize( *VAR_UnkFileHandle, NULL );

    for ( unsigned int n = 0; n < MAX_GTA_IMG_ARCHIVES; n++ )
    {
        IMGFile& file = imgArchives[n];

        if ( file.name[0] == '\0' )
            break;

        if ( !file.isNotPlayerImg )
            continue;

        //CStreaming__LoadArchive( file, n );
        ((void (__cdecl*)( const char *, unsigned int ))0x005B6170)( file.name, n );
    }

    *(unsigned int*)0x008E4C64 = 0;
    *VAR_NumResourceEntries = *VAR_NumResourceEntries / 2048;
}

CStreamingSA::CStreamingSA()
{
    // Initialize the accelerated streaming structures
    memset( g_colReplacement, 0, sizeof(g_colReplacement) );
    memset( g_originalCollision, 0, sizeof(g_originalCollision) );

    // Hook the model requested
    HookInstall( FUNC_CStreaming__RequestModel, (DWORD)HOOK_CStreaming__RequestModel, 6 );
    HookInstall( 0x004089A0, (DWORD)HOOK_CStreaming__FreeModel, 6 );
    HookInstall( 0x005B82C0, (DWORD)HOOK_CStreaming__LoadArchives, 5 );
    HookInstall( 0x00410730, (DWORD)FreeCOLLibrary, 5 );
    HookInstall( 0x0040C6B0, (DWORD)LoadModel, 5 );
}

CStreamingSA::~CStreamingSA()
{
}

void CStreamingSA::RequestModel( unsigned short id, unsigned int flags )
{
    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;

    if ( id > MAX_MODELS-1 )
        return;

    switch( info->m_eLoading )
    {
    case MODEL_LOADING:
        if ( flags & 0x10 && !(info->m_flags & 0x10) )
        {
            (*(DWORD*)VAR_NUMPRIOMODELS)++;

            info->m_flags |= FLAG_PRIORITY;
        } 
        break;
    case MODEL_UNAVAILABLE:
        // We only call the client if we request unavailable items
        if ( streamingRequestCallback )
            streamingRequestCallback( id );

        // Model support fix: quick load a model if we already have it for replacement; prevents memory leak and boosts speed
        if ( id < DATA_TEXTURE_BLOCK )
        {
            CBaseModelInfoSAInterface *minfo = ppModelInfo[id];

            // Fix for invalid model requests
            if ( !minfo )
                return;

            eRwType rwType = minfo->GetRwModelType();

            if ( CRwObjectSA *obj = g_replObjectNative[id] )
            {
                // Apply the model
                switch( rwType )
                {
                case RW_ATOMIC:
                    ((CAtomicModelInfoSA*)minfo)->SetAtomic( ((CRpAtomicSA*)obj)->CreateInstance( id ) ); // making a copy is essential for model instance isolation
                    break;
                case RW_CLUMP:
                    if ( CColModelSA *col = g_colReplacement[id] )
                        col->Apply( id );

                    ((CClumpModelInfoSAInterface*)minfo)->SetClump( RpClumpClone( (RpClump*)obj->GetObject() ) );
                    break;
                }

                info->m_eLoading = MODEL_LOADED;
                return;
            }
        }
        break;
    default:
        flags &= ~FLAG_PRIORITY;
        break;
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

        __asm
        {
            mov eax,ds:[0x008E4C60]
            push eax
            mov ecx,info
            mov eax,0x00407480
            call eax
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
    default:
        goto reload;
    case MODEL_UNAVAILABLE:
        break;
    }

    if ( id < DATA_TEXTURE_BLOCK )
    {
        CBaseModelInfoSAInterface *model = ppModelInfo[id];
        int animIndex = model->GetAnimFileIndex();

        // Accelerate our textures, yay
        RequestModel( model->m_textureDictionary + DATA_TEXTURE_BLOCK, flags );

        // Get animation if necessary
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

    __asm
    {
        mov eax,ds:[0x008E4C58]
        push eax
        mov ecx,info
        mov eax,0x00407480
        call eax
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

            // Model management fix: we unlink the collision so GTA:SA does not destroy it during
            // RwObject destruction
            if ( g_colReplacement[id] && model->GetRwModelType() == RW_CLUMP )
                model->m_pColModel = NULL;

            model->DeleteRwObject();

            switch( model->GetModelType() )
            {
            case MODEL_ATOMIC:
#ifdef _DEBUG
                OutputDebugString( SString( "deleted mesh-type model %u\n", id ) );
#endif
                break;
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

            // Model support bugfix: if we have a replacement for this model, we should not
            // bother about management in the CStreaming class, so quit here
            if ( g_replObjectNative[id] )
                goto customJump;
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
            FreeCOLLibrary( id - 25000 );
        }
        else if ( id < 25511 )
        {
            dwFunc = 0x00404B20;

            // This function destroys buildings/IPLs!
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

customJump:
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
        // This appears to be a very rare scenario, probably an exception handling system
        if ( id < DATA_TEXTURE_BLOCK )
            RwFlushLoader();
        else if ( id < DATA_TEXTURE_BLOCK + MAX_TXD )
            (*ppTxdPool)->Get( id - DATA_TEXTURE_BLOCK )->Deallocate();
        else if ( id < 25255 )
            FreeCOLLibrary( id - 25000 );
        else if ( id < 25511 )
            ( (void (*)( unsigned int model ))0x00404B20 )( id - 25255 );
        else if ( id >= DATA_ANIM_BLOCK && id < 25755 )
            pGame->GetAnimManager()->RemoveAnimBlock( id - DATA_ANIM_BLOCK );
        else if ( id >= 26230 )
            ( (void (__stdcall*)( unsigned int model ))0x004708E0 )( id - 26230 );
    }

    info->m_eLoading = MODEL_UNAVAILABLE;

    if ( streamingFreeCallback )
        streamingFreeCallback( id );
}

void CStreamingSA::LoadAllRequestedModels( bool onlyPriority )
{
    DWORD dwFunction = FUNC_LoadAllRequestedModels;
    _asm
    {
        movzx   eax,onlyPriority
        push    eax
        call    dwFunction
        add     esp, 4
    }
}

bool CStreamingSA::HasModelLoaded( unsigned int id )
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

bool CStreamingSA::IsModelLoading( unsigned int id )
{
    return ((CModelLoadInfoSA*)(ARRAY_CModelLoadInfo) + id)->m_eLoading == MODEL_LOADING;
}

void CStreamingSA::WaitForModel( unsigned int id )
{
    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;

    if ( id > MAX_MODELS-1 )
        return;
}

void CStreamingSA::RequestAnimations( int idx, unsigned int flags )
{
    idx += DATA_ANIM_BLOCK;
    RequestModel( idx, flags );
}

bool CStreamingSA::HaveAnimationsLoaded( int idx )
{
    idx += DATA_ANIM_BLOCK;
    return HasModelLoaded( idx );
}

bool CStreamingSA::HasVehicleUpgradeLoaded( int model )
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

void CStreamingSA::RequestSpecialModel( unsigned short model, const char *tex, unsigned int channel )
{
    DWORD dwFunc = FUNC_CStreaming_RequestSpecialModel;
    _asm
    {
        movzx   eax,channel
        push    eax
        push    tex
        push    model
        call    dwFunc
        add     esp, 0xC
    }
}

void CStreamingSA::SetRequestCallback( streamingRequestCallback_t callback )
{
    streamingRequestCallback = callback;
}

void CStreamingSA::SetFreeCallback( streamingFreeCallback_t callback  )
{
    streamingFreeCallback = callback;
}