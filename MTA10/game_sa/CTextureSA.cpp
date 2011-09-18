/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.cpp
*  PURPOSE:     RenderWare texture extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

CTextureSA::CTextureSA( RwTexture *tex )
{
    
}

CTextureSA::~CTextureSA()
{
    RwTextureDestroy( m_texture );
}

const char* CTextureSA::GetName()
{
    return m_texture->name;
}

unsigned int CTextureSA::GetHash()
{
    return pGame->GetKeyGen()->GetUppercaseKey( m_texture->name );
}

bool CTextureSA::IsImported()
{
    return false;
}

bool CTextureSA::Import( unsigned short id )
{
    return false;
}

bool CTextureSA::Remove( unsigned short id )
{
    return false;
}