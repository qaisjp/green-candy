/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CAnimBlockSA.h
*  PURPOSE:     Header file for animation block class
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CAnimBlockSA_H
#define __CAnimBlockSA_H

#include <game/CAnimBlock.h>
#include "Common.h"

class CAnimBlendAssocGroupSA;

#define ARRAY_AnimBlock             0x00B5D4C0

class CAnimBlockSAInterface // 32 bytes
{
public:
    int                 GetIndex ( void );

    char                m_name[16];
    bool                m_loaded;
    BYTE                m_pad[1];
    unsigned short      m_references;
    BYTE                m_pad2[12];
};

class CAnimBlockSA : public CAnimBlock
{
    friend class CAnimBlendAssocGroupSA;
public:
                                        CAnimBlockSA            ( CAnimBlockSAInterface * pInterface )     { m_pInterface = pInterface; }

    CAnimBlockSAInterface *             GetInterface            ( void )    { return m_pInterface; }
    char *                              GetName                 ( void )    { return m_pInterface->szName; }
    int                                 GetIndex                ( void )    { return m_pInterface->GetIndex (); }
    void                                AddRef                  ( void )    { m_pInterface->usRefs++; }
    unsigned short                      GetRefs                 ( void )    { return m_pInterface->usRefs; }
    bool                                IsLoaded                ( void )    { return m_pInterface->bLoaded; }

protected:
    CAnimBlockSAInterface *             m_pInterface;
};

#endif
