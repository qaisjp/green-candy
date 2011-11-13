/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CModelInfo.h
*  PURPOSE:     Entity model info interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_MODELINFO
#define __CGAME_MODELINFO

#include <CVector.h>

#include "CColModel.h"
class CPedModelInfo;

class CBoundingBox
{
public:
    CVector vecBoundMin;
    CVector vecBoundMax;
    CVector vecBoundOffset;
    float   fRadius;
};

enum eVehicleUpgradePosn
{
    VEHICLE_UPGRADE_POSN_BONET = 0,
    VEHICLE_UPGRADE_POSN_VENT,
    VEHICLE_UPGRADE_POSN_SPOILER,
    VEHICLE_UPGRADE_POSN_SIDESKIRT,
    VEHICLE_UPGRADE_POSN_FRONT_BULLBARS,
    VEHICLE_UPGRADE_POSN_REAR_BULLBARS,
    VEHICLE_UPGRADE_POSN_HEADLIGHTS,
    VEHICLE_UPGRADE_POSN_ROOF,
    VEHICLE_UPGRADE_POSN_NITRO,
    VEHICLE_UPGRADE_POSN_HYDRAULICS,
    VEHICLE_UPGRADE_POSN_STEREO,
    VEHICLE_UPGRADE_POSN_UNKNOWN,
    VEHICLE_UPGRADE_POSN_WHEELS,
    VEHICLE_UPGRADE_POSN_EXHAUST,
    VEHICLE_UPGRADE_POSN_FRONT_BUMPER,
    VEHICLE_UPGRADE_POSN_REAR_BUMPER,
    VEHICLE_UPGRADE_POSN_MISC,
};

class CModelInfo
{
public:
    virtual class CBaseModelInfoSAInterface *     GetInterface            ( void ) = 0;

    virtual unsigned short  GetModel                () = 0;
    virtual bool            IsBoat                  () = 0;
    virtual bool            IsCar                   () = 0;
    virtual bool            IsTrain                 () = 0;
    virtual bool            IsHeli                  () = 0;
    virtual bool            IsPlane                 () = 0;
    virtual bool            IsBike                  () = 0;
    virtual bool            IsFakePlane             () = 0;
    virtual bool            IsMonsterTruck          () = 0;
    virtual bool            IsQuadBike              () = 0;
    virtual bool            IsBmx                   () = 0;
    virtual bool            IsTrailer               () = 0;
    virtual bool            IsVehicle               () = 0;

    virtual char*           GetNameIfVehicle        () = 0;

    virtual VOID            Request                 ( bool bAndLoad = false, bool bWaitForLoad = false, bool bHighPriority = false ) = 0;
    virtual VOID            Remove                  () = 0;
    virtual BYTE            GetLevelFromPosition    ( CVector * vecPosition ) = 0;
    virtual BOOL            IsLoaded                () = 0;
    virtual BYTE            GetFlags                () = 0;
    virtual CBoundingBox*   GetBoundingBox          () = 0;
    virtual bool            IsValid                 () = 0;
    virtual unsigned short  GetTextureDictionaryID  () = 0;
    virtual float           GetLODDistance          () = 0;
    virtual void            SetLODDistance          ( float fDistance ) = 0;
    virtual void            RestreamIPL             () = 0;

    virtual void            AddRef                  ( bool bWaitForLoad, bool bHighPriority = false ) = 0;
    virtual void            RemoveRef               ( bool bRemoveExtraGTARef = false ) = 0;
    virtual int             GetRefCount             () = 0;

    virtual float           GetDistanceFromCentreOfMassToBaseOfModel () = 0;

    // ONLY use for CVehicleModelInfos
    virtual short           GetAvailableVehicleMod  ( unsigned short usSlot ) = 0;
    virtual bool            IsUpgradeAvailable      ( eVehicleUpgradePosn posn ) = 0;
    virtual void            SetCustomCarPlateText   ( const char * szText ) = 0;
    virtual unsigned int    GetNumRemaps            ( void ) = 0;
    virtual void*           GetVehicleSuspensionData( void ) = 0;
    virtual void*           SetVehicleSuspensionData( void* pSuspensionLines ) = 0;

    // ONLY use for upgrade models
    virtual void            RequestVehicleUpgrade   ( void ) = 0;

    // ONLY use for peds
    virtual void            GetVoice                ( short* psVoiceType, short* psVoice ) = 0;
    virtual void            GetVoice                ( const char** pszVoiceType, const char** szVoice ) = 0;
    virtual void            SetVoice                ( short sVoiceType, short sVoice ) = 0;
    virtual void            SetVoice                ( const char* szVoiceType, const char* szVoice ) = 0;

    // Custom collision related functions
    virtual void            SetCustomModel          ( RpClump* pClump ) = 0;
    virtual void            RestoreOriginalModel    ( void ) = 0;
    virtual void            SetColModel             ( CColModel* pColModel ) = 0;
    virtual void            RestoreColModel         ( void ) = 0;

    // Call this to make sure the custom vehicle models are being used after a load.
    virtual void            MakeCustomModel         ( void ) = 0;
};

#endif
