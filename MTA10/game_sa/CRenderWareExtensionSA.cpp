/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareExtensionSA.cpp
*  PURPOSE:     RenderWare extension management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

RwExtensionInterface **ppExtInterface = (RwExtensionInterface**)0x00C9B920;
#define pExtInterface   (*ppExtInterface)

unsigned int *m_pNumRwExtensions = (unsigned int*)0x00C97900;
#define m_numRwExtensions   (*m_pNumRwExtensions)

void RpAtomicRenderAlpha( RpAtomic *atom, unsigned int alpha )
{
    // Fix to overcome material limit of 152 (yes, we actually reached that in GTA:United)
    RpGeometry *geom = atom->m_geometry;
    unsigned int _flags = geom->flags;
    unsigned int n;

    geom->flags |= 0x40;

    RpMaterials& mats = geom->m_materials;
    char *alphaVals = new char [mats.m_entries];

    // Store the atomic alpha values
    for ( n = 0; n < mats.m_entries; n++ )
    {
        RpMaterial& mat = *mats.m_data[n];
        unsigned char a = mat.m_color.a;

        alphaVals[n] = a;

        if ( a > alpha )
            mat.m_color.a = alpha;
    }

    // Render it
    RpAtomicRender( atom );

    // Restore values
    while ( n-- )
        mats.m_data[n]->m_color.a = alphaVals[n];

    delete alphaVals;

    geom->flags = _flags;
}

static RpAtomic* __cdecl _worldAtomicSceneCopyConstructor( RpAtomic *atom, RpAtomic *src )
{
    atom->m_scene = src->m_scene;
    return atom;
}

CRwExtensionManagerSA::CRwExtensionManagerSA()
{
    // Patch some fixes
    HookInstall( 0x00732480, (DWORD)RpAtomicRenderAlpha, 5 );

    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x007509F0, (DWORD)_worldAtomicSceneCopyConstructor, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x007509A0, (DWORD)_worldAtomicSceneCopyConstructor, 5 );
        break;
    }

    // Initialize global extensions
    RpGeometryStreamlineInit();
}

CRwExtensionManagerSA::~CRwExtensionManagerSA()
{
    // Shutdown global extensions
    RpGeometryStreamlineShutdown();
}

RwExtension* CRwExtensionManagerSA::Allocate( unsigned int rwId, unsigned int count, size_t size, unsigned int unk )
{
    unsigned int n;
    RwExtensionInterface *ext;
    RwExtension *inst;

    for (n=0; n<m_numRwExtensions; n++)
    {
        if ((ext = &pExtInterface[n])->m_id == rwId)
            break;
    }

    if (n == m_numRwExtensions)
    {
        // Probably some error handler here
        return NULL;
    }

    inst = (RwExtension*)pRwInterface->m_malloc( sizeof(RwExtension) + ext->m_structSize * count + ext->m_internalSize );

    inst->m_size = size;
    inst->m_count = count;
    inst->m_unknown = unk;

    inst->m_extension = ext;

    inst->m_data = (void*)(inst + 1);

    if ( ext->m_internalSize == 0 )
    {
        inst->m_internal = NULL;
        return inst;
    }

    inst->m_internal = (void*)((unsigned int)inst->m_data + ext->m_structSize * count);
    return inst;
}

void CRwExtensionManagerSA::Free( RwExtension *ext )
{
    // No idea if that is correct, i.e. cleanup?
    pRwInterface->m_free( ext );
}

static int RwTranslatedIsoStreamClose( void *file )
{
    delete (CFile*)file;

    return 0;
}

static int RwTranslatedStreamClose( void *file )
{
    return 0;
}

static size_t RwTranslatedStreamRead( void *file, void *buffer, size_t length )
{
    return ((CFile*)file)->Read( buffer, 1, length );
}

static size_t RwTranslatedStreamWrite( void *file, const void *buffer, size_t length )
{
    return ((CFile*)file)->Write( buffer, 1, length );
}

static void* RwTranslatedStreamSeek( void *file, unsigned int offset )
{
    size_t endOff = ((CFile*)file)->GetSize() - (size_t)((CFile*)file)->Tell();
    unsigned int roff = min( offset, endOff );

    if ( roff == 0 )
        return NULL;

    return ( ((CFile*)file)->Seek( roff, SEEK_CUR ) == 0 ) ? file : NULL;
}

RwStream* RwStreamCreateTranslated( CFile *file )
{
    if ( !file )
        return NULL;

    RwStreamTypeData data;
    data.callbackClose = RwTranslatedStreamClose;
    data.callbackRead = RwTranslatedStreamRead;
    data.callbackWrite = RwTranslatedStreamWrite;
    data.callbackSeek = RwTranslatedStreamSeek;
    data.ptr_callback = file;

    return RwStreamOpen( STREAM_TYPE_CALLBACK, STREAM_MODE_NULL, &data );
}

RwStream* RwStreamCreateIsoTranslated( CFile *file )
{
    if ( !file )
        return NULL;

    RwStreamTypeData data;
    data.callbackClose = RwTranslatedIsoStreamClose;
    data.callbackRead = RwTranslatedStreamRead;
    data.callbackWrite = RwTranslatedStreamWrite;
    data.callbackSeek = RwTranslatedStreamSeek;
    data.ptr_callback = file;

    return RwStreamOpen( STREAM_TYPE_CALLBACK, STREAM_MODE_NULL, &data );
}

RwStream* RwStreamOpenTranslated( const char *path, RwStreamMode mode )
{
    CFile *file;

    switch( mode )
    {
    case STREAM_MODE_READ:
        file = OpenGlobalStream( path, "rb" );
        break;
    case STREAM_MODE_WRITE:
        file = OpenGlobalStream( path, "wb" );
        break;
    case STREAM_MODE_APPEND:
        file = OpenGlobalStream( path, "ab" );
        break;
    default:
        return NULL;
    }

    return RwStreamCreateIsoTranslated( file );
}