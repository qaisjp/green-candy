/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwTexDictionary.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

static bool RwTexDictionaryGetFirstTexture( RwTexture *tex, RwTexture **rslt )
{
    *rslt = tex;
    return false;
}

RwTexture* RwTexDictionary::GetFirstTexture()
{
    RwTexture *tex;

    if ( ForAllTextures( RwTexDictionaryGetFirstTexture, &tex ) )
        return NULL;

    return tex;
}

RwTexture* RwTexDictionary::FindNamedTexture( const char *name )
{
    LIST_FOREACH_BEGIN( RwTexture, textures.root, TXDList )
        if ( stricmp( item->name, name ) == 0 )
            return item;
    LIST_FOREACH_END

    return NULL;
}

RwTexDictionary* RwTexDictionaryCreate( void )
{
    RwTexDictionary *txd = (RwTexDictionary*)pRwInterface->m_allocStruct( pRwInterface->m_textureManager.txdStruct, 0x30016 );

    if ( !txd )
        return NULL;

    txd->type = RW_TXD;
    txd->subtype = 0;
    txd->flags = 0;
    txd->privateFlags = 0;
    txd->parent = NULL;

    LIST_CLEAR( txd->textures.root );
    LIST_APPEND( pRwInterface->m_textureManager.globalTxd.root, txd->globalTXDs );

    // Register the txd I guess
    RwObjectRegister( (void*)0x008E23E4, txd );
    return txd;
}