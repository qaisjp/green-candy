/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CTextureManager.h
*  PURPOSE:     Texture management definitions
*               SubInterface of RenderWare
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTextureManager_H_
#define _CTextureManager_H_

// CStreamer supports 5000 due to hardcoding
#define MAX_TXD     5000

class CTextureManager
{
public:
    virtual int                 FindTxdEntry( const char *name ) const = 0;
    virtual int                 CreateTxdEntry( const char *name ) = 0;

    virtual CTexDictionary*     CreateTxd( const char *name ) = 0;
    virtual CTexDictionary*     CreateTxd( const char *name, unsigned short txdId ) = 0;

    virtual int                 LoadDictionary( const char *filename ) = 0;
    virtual int                 LoadDictionaryEx( const char *name, const char *filename ) = 0;

    virtual bool                SetCurrentTexture( unsigned short id ) = 0;

    virtual void                RemoveTxdEntry( unsigned short id ) = 0;
};

#endif