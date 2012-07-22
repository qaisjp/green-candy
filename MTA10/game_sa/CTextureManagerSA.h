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

class CTexDictionarySA;

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

    void            SetCurrent();

    RwTexDictionary*    m_txd;
    unsigned short      m_references;
    unsigned short      m_parentTxd;
    unsigned int        m_hash;
};

class CTextureManagerSA : public CTextureManager
{
    friend class CTexDictionarySA;
public:
                        CTextureManagerSA();
                        ~CTextureManagerSA();

    int                 FindTxdEntry( const char *name ) const;
    int                 CreateTxdEntry( const char *name );

    CTexDictionary*     CreateTxd( const char *name );
    CTexDictionary*     CreateTxd( const char *name, unsigned short txdId );

    int                 LoadDictionary( const char *filename );
    int                 LoadDictionaryEx( const char *name, const char *filename );

    bool                SetCurrentTexture( unsigned short id );

    void                RemoveTxdEntry( unsigned short id );

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

    // Managed textures
    typedef std::list <CTexDictionarySA*> txdList_t;
    txdList_t                               m_texDicts;

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