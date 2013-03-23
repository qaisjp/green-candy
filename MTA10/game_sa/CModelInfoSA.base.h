/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelInfoSA.base.h
*  PURPOSE:     Header file for entity model information handler class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODELINFO_BASE_
#define _MODELINFO_BASE_

#if 0
class CClumpModelInfo_SA_VTBL
{
    DWORD           SetClump;                       // (RpClump*)
};

class CWeaponModelInfo_SA_VTBL: public CClumpModelInfo_SA_VTBL
{
};
#endif

class CAtomicModelInfoSA;
class CDamageAtomicModelInfoSA;
class CLODAtomicModelInfoSA;

enum eModelType : unsigned char
{
    MODEL_ATOMIC = 1,
    MODEL_VEHICLE = 6,
    MODEL_PED
};

namespace ModelInfo
{
    struct timeInfo
    {
        BYTE                m_pad[2];
        unsigned short      m_model;
    };
};

// Abstract class for every model info class
class CBaseModelInfoSA abstract
{
public:
    virtual                                             ~CBaseModelInfoSA()     {}                              // 0

    virtual CAtomicModelInfoSA* __thiscall              GetAtomicModelInfo          ( void ) = 0;               // 4
    virtual CDamageAtomicModelInfoSA* __thiscall        GetDamageAtomicModelInfo    ( void ) = 0;               // 8
    virtual CLODAtomicModelInfoSA* __thiscall           GetLODAtomicModelInfo       ( void ) = 0;               // 12
    virtual eModelType __thiscall                       GetModelType                ( void ) = 0;               // 16
    virtual ModelInfo::timeInfo* __thiscall             GetTimeInfo                 ( void ) = 0;               // 20
    virtual void __thiscall                             Init                        ( void ) = 0;               // 24
    virtual void __thiscall                             Shutdown                    ( void ) = 0;               // 28
    virtual void __thiscall                             DeleteRwObject              ( void ) = 0;               // 32
    virtual eRwType __thiscall                          GetRwModelType              ( void ) = 0;               // 36
    virtual RwObject* __thiscall                        CreateRwObjectEx            ( RwMatrix& mat ) = 0;      // 40
    virtual RwObject* __thiscall                        CreateRwObject              ( void ) = 0;               // 44
    virtual void __thiscall                             SetAnimFile                 ( const char *name ) = 0;   // 48
    virtual void __thiscall                             ConvertAnimFileIndex        ( void ) = 0;               // 52
    virtual int __thiscall                              GetAnimFileIndex            ( void ) = 0;               // 56
};

#define RENDER_PRERENDERED          0x0001
#define RENDER_LAST                 0x0002
#define RENDER_ADDITIVE             0x0004
#define RENDER_NOSHADOW             0x0008
#define RENDER_NOCULL               0x0010
#define RENDER_LOD                  0x0020
#define RENDER_BACKFACECULL         0x0040
#define RENDER_COLMODEL             0x0080
#define RENDER_STATIC               0x0100
#define RENDER_NOSKELETON           0x0200

#define COLL_AUDIO                  0x0001
#define COLL_SWAYINWIND             0x0002
#define COLL_STREAMEDWITHMODEL      0x0004
#define COLL_NOCOLLFLYER            0x0008
#define COLL_COMPLEX                0x0010
#define COLL_WETROADREFLECT         0x0020

// Interface which provides common functionality to model info deriviations.
class CBaseModelInfoSAInterface abstract : public CBaseModelInfoSA
{
public:
    CBaseModelInfoSAInterface                               ( void );
    ~CBaseModelInfoSAInterface();

    CAtomicModelInfoSA*         GetAtomicModelInfo          ( void );
    CDamageAtomicModelInfoSA*   GetDamageAtomicModelInfo    ( void );
    CLODAtomicModelInfoSA*      GetLODAtomicModelInfo       ( void );
    void                        Init                        ( void );
    void                        Shutdown                    ( void );
    void __thiscall             SetCollision                ( CColModelSAInterface *col, bool isDynamic );
    void __thiscall             DeleteCollision             ( void );
    void                        DereferenceTXD              ( void );
    ModelInfo::timeInfo*        GetTimeInfo                 ( void );
    void                        Reference                   ( void );
    void                        Dereference                 ( void );

    void __thiscall             SetColModel                 ( CColModelSAInterface *col, bool isDynamic );
    bool                        IsDynamicCol                ( void ) const              { return IS_FLAG( m_renderFlags, RENDER_COLMODEL ); }
    void __thiscall             UnsetColModel               ( void );

    unsigned short              GetFlags                    ( void );

    RwObject*                   GetRwObject                 ( void )                    { return m_rwObject; }
    const RwObject*             GetRwObject                 ( void ) const              { return m_rwObject; }

    unsigned int                m_hash;                     // +4       Generated by CKeyGen::GetUppercaseKey(char const *) called by CBaseModelInfo::SetModelName(char const *)
    unsigned short              m_numberOfRefs;             // +8
    short                       m_textureDictionary;        // +10
    unsigned char               m_alpha;                    // +12

    unsigned char               m_num2dfx;                  // +13
    short                       m_effectID;                 // +14      Something with 2d effects

    short                       m_dynamicIndex;             // +16      Index of attributes in g_dynObjData

    union
    {
        struct
        {
            unsigned short      m_renderFlags : 10;         // 18
            unsigned short      m_collFlags : 6;
        };
        struct
        {
            unsigned short      m_flags;                    // 18
        };
    };

    CColModelSAInterface*       m_pColModel;                // +20      CColModel: public CBoundingBox

    float                       m_lodDistance;              // +24      Scaled is this value multiplied with flt_B6F118

    RwObject*                   m_rwObject;                 // 28

    // CClumpModelInfo::SetCollisionWasStreamedWithModel(unsigned int)
    // CAtomicModelInfo::SetDontCollideWithFlyer(unsigned int)
    // CClumpModelInfo::SetHasComplexHierarchy(unsigned int)
    // CAtomicModelInfo::SetWetRoadReflection(unsigned int)

    // CWeaponModelInfo:
    // +36 = Weapon info as int
};

void ModelInfoBase_Init();
void ModelInfoBase_Shutdown();

#endif //_MODELINFO_BASE_