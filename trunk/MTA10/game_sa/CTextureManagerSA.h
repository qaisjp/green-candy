/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTextureManagerSA.h
*  PURPOSE:     Internal texture management definitions
*               SubInterface of RenderWare
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTextureManagerSA_H_
#define _CTextureManagerSA_H_

#include <game/CTextureManager.h>

class CTxdInstanceSA
{
    friend class CTextureManagerSA;
public:
                    CTxdInstanceSA( const char *name );
                    ~CTxdInstanceSA();

    void*   operator new( size_t );
    void    operator delete( void *ptr );

    bool            LoadTXD( const char *filename );
    unsigned short  InitParent();
    void            Reference();
    void            Dereference();

    RwTexDictionary*    m_txd;
    unsigned short      m_references;
    unsigned short      m_parentTxd;
    unsigned int        m_hash;
};

class CTextureManagerSA
{
public:
                        CTextureManagerSA();
                        ~CTextureManagerSA();

    int                 FindTxdEntry( const char *name );
    int                 CreateTxdEntry( const char *name );

    int                 LoadDictionary( const char *filename );
    int                 LoadDictionaryEx( const char *name, const char *filename );

    // Adds textures into the TXD of a model, eventually making a copy of each texture first
    void                ModelInfoTXDAddTextures     ( std::list < RwTexture* >& textures, unsigned short usModelID, bool bMakeCopy = true, std::list < RwTexture* >* pReplacedTextures = NULL, std::list < RwTexture* >* pAddedTextures = NULL, bool bAddRef = true );

    // Removes texture from the TXD of a model, eventually destroying each texture
    void                ModelInfoTXDRemoveTextures  ( std::list < RwTexture* >& textures, unsigned short usModelID, bool bDestroy = true, bool bKeepRaster = false, bool bRemoveRef = true );

    // Reads and parses a TXD file specified by a path (szTXD)
    RwTexDictionary *   ReadTXD                     ( const char *szTXD );

    // Destroys a TXD instance
    void                DestroyTXD                  ( RwTexDictionary * pTXD );

    // Destroys a texture
    void                DestroyTexture              ( RwTexture* pTex );

    ushort              GetTXDIDForModelID          ( ushort usModelID );
    void                InitWorldTextureWatch       ( PFN_WATCH_CALLBACK pfnWatchCallback );
    bool                AddWorldTextureWatch        ( CSHADERDUMMY* pShaderData, const char* szMatch, float fOrderPriority );
    void                RemoveWorldTextureWatch     ( CSHADERDUMMY* pShaderData, const char* szMatch );
    void                RemoveWorldTextureWatchByContext ( CSHADERDUMMY* pShaderData );
    void                PulseWorldTextureWatch      ( void );
    void                GetModelTextureNames        ( std::vector < SString >& outNameList, ushort usModelID );
    void                GetTxdTextures              ( std::vector < RwTexture* >& outTextureList, ushort usTxdId );
    const SString&      GetTextureName              ( CD3DDUMMY* pD3DData );

private:
    static void         RwTexDictionaryRemoveTexture( RwTexDictionary* pTXD, RwTexture* pTex );
    static short        CTxdStore_GetTxdRefcount    ( unsigned short usTxdID );
    static bool         ListContainsNamedTexture    ( std::list < RwTexture* >& list, const char* szTexName );
    static bool         StaticGetTextureCB          ( RwTexture* texture, std::vector < RwTexture* >* pTextureList );
    void                AddActiveTexture            ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData );
    void                RemoveTxdActiveTextures     ( ushort usTxdId );
    void                FindNewAssociationForTexInfo( STexInfo* pTexInfo );
    STexInfo*           CreateTexInfo               ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData );
    void                OnDestroyTexInfo            ( STexInfo* pTexInfo );
    SShadInfo*          CreateShadInfo              ( CSHADERDUMMY* pShaderData, const SString& strTextureName, float fOrderPriority );
    void                OnDestroyShadInfo           ( SShadInfo* pShadInfo );
    void                MakeAssociation             ( SShadInfo* pShadInfo, STexInfo* pTexInfo );
    void                BreakAssociation            ( SShadInfo* pShadInfo, STexInfo* pTexInfo );

    // Watched world textures
    std::list < SShadInfo >                 m_ShadInfoList;
    std::list < STexInfo >                  m_TexInfoList;

    std::map < SString, STexInfo* >         m_UniqueTexInfoMap;
    std::map < CD3DDUMMY*, STexInfo* >      m_D3DDataTexInfoMap;
    std::map < SString, SShadInfo* >        m_UniqueShadInfoMap;
    std::multimap < float, SShadInfo* >     m_orderMap;

    PFN_WATCH_CALLBACK                      m_pfnWatchCallback;
};

#endif