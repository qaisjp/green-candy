/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelInfoSA.h
*  PURPOSE:     Header file for entity model information handler class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_MODELINFO
#define __CGAMESA_MODELINFO

#include <game/CModelInfo.h>
#include <game/Common.h>

#include "CColModelSA.h"
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

class CAtomicModelInfo_SA_VTBL
{
    DWORD           SetAtomic;                      // (RpAtomic*)
};

class CDamageAtomicModelInfo_SA_VTBL: public CAtomicModelInfo_SA_VTBL
{
};

class CLodAtomicModelInfo_SA_VTBL: public CAtomicModelInfo_SA_VTBL
{
};

class CTimeModelInfo_SA_VTBL: public CAtomicModelInfo_SA_VTBL
{
};

class CLodTimeModelInfo_SA_VTBL: public CLodAtomicModelInfo_SA_VTBL
{
};

class CClumpModelInfo_SA_VTBL
{
    DWORD           SetClump;                       // (RpClump*)
};

class CWeaponModelInfo_SA_VTBL: public CClumpModelInfo_SA_VTBL
{
};

class CPedModelInfo_SA_VTBL: public CClumpModelInfo_SA_VTBL
{
};

class CVehicleModelInfo_SA_VTBL: public CClumpModelInfo_SA_VTBL
{
};

class CAtomicModelInfoSA;
class CDamageAtomicModelInfoSA;
class CLODAtomicModelInfoSA;

enum eModelType
{
    MODEL_VEHICLE = 6,
    MODEL_PED
};

class CBaseModelInfoSA
{
public:
    virtual                             ~CBaseModelInfoSA()
    {
    }

    virtual CAtomicModelInfoSA* __thiscall              GetAtomicModelInfo() = 0;
    virtual CDamageAtomicModelInfoSA* __thiscall        GetDamageAtomicModelInfo() = 0;
    virtual CLODAtomicModelInfoSA* __thiscall           GetLODAtomicModelInfo() = 0;
    virtual eModelType __thiscall                       GetModelType() = 0;
    virtual unsigned int __thiscall                     GetTimeInfo() = 0;
    virtual void __thiscall                             Init() = 0;
    virtual void __thiscall                             Shutdown() = 0;
    virtual void __thiscall                             DeleteRwObject() = 0;
    virtual eRwType __thiscall                          GetRwModelType() = 0;
    virtual RwObject* __thiscall                        CreateRwObjectEx( int rwTag ) = 0;
    virtual RwObject* __thiscall                        CreateRwObject() = 0;
    virtual bool __thiscall                             SetAnimFile( const char *name ) = 0;
    virtual void __thiscall                             ConvertAnimFileIndex() = 0;
    virtual int __thiscall                              GetAnimFileIndex() = 0;
};

#define RENDER_COLMODEL             0x0001
#define RENDER_BACKFACECULL         0x0002
#define RENDER_LOD                  0x0004
#define RENDER_NOSHADOW             0x0008
#define RENDER_NOZBUFFER            0x0010
#define RENDER_ADDITIVE             0x0020
#define RENDER_LAST                 0x0040
#define RENDER_PRERENDERED          0x0080

#define COLL_UNK1                   0x0001
#define COLL_UNK2                   0x0002
#define COLL_UNK3                   0x0004
#define COLL_SWAYINWIND             0x0008
#define COLL_STREAMEDWITHMODEL      0x0010
#define COLL_NOCOLLFLYER            0x0020
#define COLL_COMPLEX                0x0040
#define COLL_WETROADREFLECT         0x0080

class CBaseModelInfoSAInterface : public CBaseModelInfoSA
{
public:
    CBaseModelInfoSAInterface();
    ~CBaseModelInfoSAInterface();

    CAtomicModelInfoSA*         GetAtomicModelInfo();
    CDamageAtomicModelInfoSA*   GetDamageAtomicModelInfo();
    CLODAtomicModelInfoSA*      GetLODAtomicModelInfo();
    unsigned int                GetTimeInfo();
    void                        Reference();
    void                        Dereference();
    void                        DeleteTextures();

    unsigned long               m_ulHashKey;                  // +4   Generated by CKeyGen::GetUppercaseKey(char const *) called by CBaseModelInfo::SetModelName(char const *)
    unsigned short              m_numberOfRefs: 16;         // +8
    short                       m_textureDictionary: 16;    // +10
    unsigned char               m_alpha: 8;                 // +12

    unsigned char               m_numberOf2DEffects: 8;        // +13
    short                       m_effectID: 16;              // +14     Something with 2d effects

    short                       m_dynamicIndex;

    unsigned char               m_renderFlags;

    unsigned char               m_collFlags;

    CColModelSAInterface*       m_pColModel;                    // +20      CColModel: public CBoundingBox

    float                       m_lodDistance;                  // +24      Scaled is this value multiplied with flt_B6F118

    // CClumpModelInfo::SetCollisionWasStreamedWithModel(unsigned int)
    // CAtomicModelInfo::SetDontCollideWithFlyer(unsigned int)
    // CClumpModelInfo::SetHasComplexHierarchy(unsigned int)
    // CAtomicModelInfo::SetWetRoadReflection(unsigned int)

    // CWeaponModelInfo:
    // +36 = Weapon info as int

    // CPedModelInfo:
    // +36 = Motion anim group (AssocGroupID, long)
    // +40 = Default ped type (long)
    // +44 = Default ped stats (ePedStats)
    // +48 = Can drive cars (byte)
    // +50 = Ped flags (short)
    // +52 = Hit col model (CColModel*)
    // +56 = First radio station
    // +57 = Second radio station
    // +58 = Race (byte)
    // +60 = Audio ped type (short)
    // +62 = First voice
    // +64 = Last voice
    // +66 = Next voice (short)
};

class CClumpModelInfoSAInterface : public CBaseModelInfoSAInterface
{
public:
    virtual CColModelSAInterface*       GetCollision() = 0;
    virtual void __thiscall             SetClump( RpClump *clump );

    void                Init();
    void                Shutdown();
    void                DeleteRwObject();
    eRwType             GetRwModelType();
    void                DeleteCollision();
    RpClump*            CreateRwObjectEx( int rwTag );
    RpClump*            CreateRwObject();

    RpClump*                m_rwClump;                      // 28
};

class CAtomicModelInfoSA : public CBaseModelInfoSAInterface
{
public:
    virtual void __thiscall             SetAtomic( RpAtomic *atomic );

    RpAtomic*                           m_rpAtomic;     // 28
};

/**
 * \todo Someone move GetLevelFromPosition out of here or delete it entirely please
 */


class CModelInfoSA : public CModelInfo
{
protected:
    CBaseModelInfoSAInterface *     m_pInterface;
    unsigned short                  m_modelID;
    DWORD                           m_dwReferences;
    CColModel*                      m_pCustomColModel;
    CColModelSAInterface*           m_pOriginalColModelInterface;
    RpClump*                        m_pCustomClump;
    static std::map < unsigned short, int > ms_RestreamTxdIDMap;

public:
                                    CModelInfoSA            ( void );
                                    CModelInfoSA            ( unsigned short );

    CBaseModelInfoSAInterface *     GetInterface             ( void );
    CPedModelInfoSAInterface *      GetPedModelInfoInterface ( void )              { return reinterpret_cast < CPedModelInfoSAInterface * > ( GetInterface () ); }

    unsigned short                  GetModel                ( void )               { return m_modelID; }

    bool                            IsBoat                  ( void );
    bool                            IsCar                   ( void );
    bool                            IsTrain                 ( void );
    bool                            IsHeli                  ( void );
    bool                            IsPlane                 ( void );
    bool                            IsBike                  ( void );
    bool                            IsFakePlane             ( void );
    bool                            IsMonsterTruck          ( void );
    bool                            IsQuadBike              ( void );
    bool                            IsBmx                   ( void );
    bool                            IsTrailer               ( void );
    bool                            IsVehicle               ( void );
    bool                            IsUpgrade               ( void );

    char *                          GetNameIfVehicle        ( void );

    VOID                            Request                 ( bool bAndLoad = false, bool bWaitForLoad = false, bool bHighPriority = false );
    VOID                            Remove                  ( void );
    BYTE                            GetLevelFromPosition    ( CVector * vecPosition );
    BOOL                            IsLoaded                ( void );
    void                            InternalRemoveGTARef    ( void );
    BYTE                            GetFlags                ( void );
    CBoundingBox *                  GetBoundingBox          ( void );
    bool                            IsValid                 ( void );
    float                           GetDistanceFromCentreOfMassToBaseOfModel ( void );
    unsigned short                  GetTextureDictionaryID  ( void );
    void                            SetTextureDictionaryID  ( unsigned short usID );
    float                           GetLODDistance          ( void );
    void                            SetLODDistance          ( float fDistance );
    void                            RestreamIPL             ( void );
    static void                     StaticFlushPendingRestreamIPL ( void );

    void                            AddRef                  ( bool bWaitForLoad, bool bHighPriority = false );
    int                             GetRefCount             ( void );
    void                            RemoveRef               ( bool bRemoveExtraGTARef = false );
    void                            MaybeRemoveExtraGTARef  ( void );
    void                            DoRemoveExtraGTARef     ( void );

    // CVehicleModelInfo specific
    short                           GetAvailableVehicleMod  ( unsigned short usSlot );
    bool                            IsUpgradeAvailable      ( eVehicleUpgradePosn posn );
    void                            SetCustomCarPlateText   ( const char * szText );
    unsigned int                    GetNumRemaps            ( void );
    void*                           GetVehicleSuspensionData( void );
    void*                           SetVehicleSuspensionData( void* pSuspensionLines );

    // Upgrades only!
    void                            RequestVehicleUpgrade   ( void );

    // ONLY use for peds
    void                            GetVoice                ( short* psVoiceType, short* psVoice );
    void                            GetVoice                ( const char** pszVoiceType, const char** szVoice );
    void                            SetVoice                ( short sVoiceType, short sVoice );
    void                            SetVoice                ( const char* szVoiceType, const char* szVoice );

    // Custom collision related functions
    void                            SetCustomModel          ( RpClump* pClump );
    void                            RestoreOriginalModel    ( void );
    void                            SetColModel             ( CColModel* pColModel );
    void                            RestoreColModel         ( void );
    void                            MakeCustomModel         ( void );

    inline void                     SetModelID              ( unsigned short id ) { m_modelID = id; }

    void                            MakePedModel            ( char * szTexture );
};

#endif