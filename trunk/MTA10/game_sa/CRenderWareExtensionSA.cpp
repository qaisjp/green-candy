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