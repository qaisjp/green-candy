/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareExtensionSA.h
*  PURPOSE:     RenderWare extension management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CRenderWareExtensionSA_H_
#define _CRenderWareExtensionSA_H_

#define MAX_RW_EXTENSIONS           16

class CRwExtensionManagerSA
{
public:
                        CRwExtensionManagerSA();
                        ~CRwExtensionManagerSA();

    // todo: Possibility to register extensions

    RwExtension*        Allocate( unsigned int extId, unsigned int count, size_t size, unsigned int unk );
    void*               Free( RwExtension *ext );

    unsigned int        GetNumExtensions();
};

#endif