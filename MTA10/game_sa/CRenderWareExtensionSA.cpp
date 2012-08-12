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

CRwExtensionManagerSA::CRwExtensionManagerSA()
{

}

CRwExtensionManagerSA::~CRwExtensionManagerSA()
{

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

static int RwTranslatedStreamSeek( void *file, unsigned int offset )
{
    return ((CFile*)file)->Seek( offset, SEEK_SET );
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