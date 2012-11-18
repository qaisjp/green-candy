/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelInfoSA.h
*  PURPOSE:     Header file for entity model information handler class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_MODELINFO
#define __CGAMESA_MODELINFO

#include <game/CModelInfo.h>
#include <game/Common.h>

#include "CRenderWareSA.h"
class CPedModelInfoSA;
class CPedModelInfoSAInterface;

#define     ARRAY_ModelLoaded               0x8E4CD0 // ##SA##

#define     FUNC_CStreaming__HasModelLoaded 0x4044C0

// CModelInfo/ARRAY_ModelInfo __thiscall to load/replace vehicle models
#define     FUNC_LoadVehicleModel           0x4C95C0

#define     DWORD_AtomicsReplacerModelID    0xB71840
#define     FUNC_AtomicsReplacer            0x537150

#define     ARRAY_ModelInfo                 0xA9B0C8
#define     CLASS_CText                     0xC1B340
#define     FUNC_CText_Get                  0x6A0050
#define     FUNC_GetModelFlags              0x4044E0
#define     FUNC_GetBoundingBox             0x4082F0

#define     FUNC_RemoveRef                  0x4C4BB0  
#define     FUNC_IsVehicleModelType         0x4c5c80

#define     FUNC_FlushRequestList           0x40E4E0

#define     FUNC_HasVehicleUpgradeLoaded    0x407820
#define     FUNC_RequestVehicleUpgrade      0x408C70

#define     FUNC_CVehicleModelInfo__GetNumRemaps        0x4C86B0

#define     FUNC_SetColModel                0x4C4BC0
#define     FUNC_AddPedModel                0x4c67a0
#define     VAR_CTempColModels_ModelPed1    0x968DF0

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

// Helpful compiler hacking tool :)
template <class funcType>
DWORD h_memFunc( funcType func )
{
    funcType f = func;
    return (DWORD)*(void**)&f;
}

class CBaseModelInfoSA abstract
{
public:
    virtual                             ~CBaseModelInfoSA()     {}

    virtual CAtomicModelInfoSA* __thiscall              GetAtomicModelInfo() = 0;               // 4
    virtual CDamageAtomicModelInfoSA* __thiscall        GetDamageAtomicModelInfo() = 0;         // 8
    virtual CLODAtomicModelInfoSA* __thiscall           GetLODAtomicModelInfo() = 0;            // 12
    virtual eModelType __thiscall                       GetModelType() = 0;                     // 16
    virtual unsigned int __thiscall                     GetTimeInfo() = 0;                      // 20
    virtual void __thiscall                             Init() = 0;                             // 24
    virtual void __thiscall                             Shutdown() = 0;                         // 28
    virtual void __thiscall                             DeleteRwObject() = 0;                   // 32
    virtual eRwType __thiscall                          GetRwModelType() = 0;                   // 36
    virtual RwObject* __thiscall                        CreateRwObjectEx( int rwTag ) = 0;      // 40
    virtual RwObject* __thiscall                        CreateRwObject() = 0;                   // 44
    virtual void __thiscall                             SetAnimFile( const char *name ) = 0;    // 48
    virtual void __thiscall                             ConvertAnimFileIndex() = 0;             // 52
    virtual int __thiscall                              GetAnimFileIndex() = 0;                 // 56
};

void    ModelInfo_Init();
void    ModelInfo_Shutdown();

#define RENDER_PRERENDERED          0x0001
#define RENDER_LAST                 0x0002
#define RENDER_ADDITIVE             0x0004
#define RENDER_NOZBUFFER            0x0008
#define RENDER_NOSHADOW             0x0010
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

class CBaseModelInfoSAInterface : public CBaseModelInfoSA
{
public:
    CBaseModelInfoSAInterface();
    ~CBaseModelInfoSAInterface();

    CAtomicModelInfoSA*         GetAtomicModelInfo();
    CDamageAtomicModelInfoSA*   GetDamageAtomicModelInfo();
    CLODAtomicModelInfoSA*      GetLODAtomicModelInfo();
    void                        Init();
    void                        Shutdown();
    void __thiscall             SetCollision( CColModelSAInterface *col, bool putTimed );
    void __thiscall             DeleteCollision();
    void                        DeleteTextures();
    unsigned int                GetTimeInfo();
    void                        Reference();
    void                        Dereference();

    void __thiscall             SetColModel( CColModelSAInterface *col, bool putTimed );
    void __thiscall             UnsetColModel();

    unsigned short              GetFlags();

    unsigned int                m_hash;                     // +4       Generated by CKeyGen::GetUppercaseKey(char const *) called by CBaseModelInfo::SetModelName(char const *)
    unsigned short              m_numberOfRefs;             // +8
    short                       m_textureDictionary;        // +10
    unsigned char               m_alpha;                    // +12

    unsigned char               m_num2dfx;                  // +13
    short                       m_effectID;                 // +14      Something with 2d effects

    short                       m_dynamicIndex;             // +16

    unsigned short              m_renderFlags : 10;         // 18
    unsigned short              m_collFlags : 6;

    CColModelSAInterface*       m_pColModel;                // +20      CColModel: public CBoundingBox

    float                       m_lodDistance;              // +24      Scaled is this value multiplied with flt_B6F118

    // CClumpModelInfo::SetCollisionWasStreamedWithModel(unsigned int)
    // CAtomicModelInfo::SetDontCollideWithFlyer(unsigned int)
    // CClumpModelInfo::SetHasComplexHierarchy(unsigned int)
    // CAtomicModelInfo::SetWetRoadReflection(unsigned int)

    // CWeaponModelInfo:
    // +36 = Weapon info as int
};

class CModelInfoSA : public CModelInfo
{
protected:
    CBaseModelInfoSAInterface*      m_pInterface;
    unsigned short                  m_modelID;
    DWORD                           m_dwReferences;
    CColModel*                      m_pCustomColModel;
    CColModelSAInterface*           m_pOriginalColModelInterface;
    static std::map <unsigned short, int> ms_RestreamTxdIDMap;

public:
                                    CModelInfoSA();
                                    CModelInfoSA( unsigned short id );

    CBaseModelInfoSAInterface*      GetInterface()                          { return m_pInterface; }
    CPedModelInfoSAInterface*       GetPedModelInfoInterface()              { return (CPedModelInfoSAInterface*)GetInterface(); }
    const CPedModelInfoSAInterface* GetPedModelInfoInterface() const        { return (const CPedModelInfoSAInterface*)m_pInterface; }

    unsigned short                  GetModel() const                        { return m_modelID; }

    eVehicleType                    GetVehicleType() const;
    bool                            IsBoat() const;
    bool                            IsCar() const;
    bool                            IsTrain() const;
    bool                            IsHeli() const;
    bool                            IsPlane() const;
    bool                            IsBike() const;
    bool                            IsBmx() const;
    bool                            IsTrailer() const;
    bool                            IsVehicle() const;
    bool                            IsUpgrade() const;

    bool                            IsPed() const;
    bool                            IsObject() const;

    const char*                     GetNameIfVehicle() const;

    void                            Request( bool bAndLoad = false, bool bWaitForLoad = false, bool bHighPriority = false );
    void                            Remove();
    unsigned char                   GetLevelFromPosition( const CVector& vecPosition ) const;
    bool                            IsLoaded() const;
    unsigned char                   GetFlags() const;
    const CBoundingBox*             GetBoundingBox() const;
    bool                            IsValid();
    float                           GetDistanceFromCentreOfMassToBaseOfModel() const;
    unsigned short                  GetTextureDictionaryID() const;
    void                            SetTextureDictionaryID( unsigned short usID );
    float                           GetLODDistance() const;
    void                            SetLODDistance( float fDistance );
    void                            RestreamIPL();
    static void                     StaticFlushPendingRestreamIPL();

    void                            AddRef( bool bWaitForLoad, bool bHighPriority = false );
    int                             GetRefCount() const                     { return (int)m_dwReferences; }
    void                            RemoveRef( bool bRemoveExtraGTARef = false );
    void                            MaybeRemoveExtraGTARef();
    void                            DoRemoveExtraGTARef();

    // CVehicleModelInfo specific
    short                           GetAvailableVehicleMod( unsigned short usSlot ) const;
    bool                            IsUpgradeAvailable( eVehicleUpgradePosn posn ) const;
    void                            SetCustomCarPlateText( const char *szText );
    unsigned int                    GetNumRemaps() const;
    void*                           GetVehicleSuspensionData() const;
    void*                           SetVehicleSuspensionData( void *pSuspensionLines );

    // Upgrades only!
    void                            RequestVehicleUpgrade();

    // ONLY use for peds
    void                            GetVoice( short* psVoiceType, short* psVoice ) const;
    void                            GetVoice( const char** pszVoiceType, const char** szVoice ) const;
    void                            SetVoice( short sVoiceType, short sVoice );
    void                            SetVoice( const char* szVoiceType, const char* szVoice );

    inline void                     SetModelID( unsigned short id )         { m_modelID = id; }

    void                            MakePedModel( const char *texture );
};

#endif