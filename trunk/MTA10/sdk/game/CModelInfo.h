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

enum eVehicleType : unsigned int
{
    VEHICLE_CAR,
    VEHICLE_MONSTERTRUCK,
    VEHICLE_QUADBIKE,
    VEHICLE_HELI,
    VEHICLE_PLANE,
    VEHICLE_BOAT,
    VEHICLE_TRAIN,
    VEHICLE_UNKNOWN,
    VEHICLE_FAKEPLANE,    // what is this?
    VEHICLE_BIKE,
    VEHICLE_BICYCLE,
    VEHICLE_AUTOMOBILETRAILER
};

class CBoundingBox
{
public:
    CVector vecBoundMin;
    CVector vecBoundMax;
    CVector vecBoundOffset;
    float   fRadius;
};

struct CBounds2D
{
    CVector2D   m_min;
    CVector2D   m_max;
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
    virtual unsigned short                  GetModel() const = 0;

    virtual eVehicleType                    GetVehicleType() const = 0;
    virtual bool                            IsBoat() const = 0;
    virtual bool                            IsCar() const = 0;
    virtual bool                            IsTrain() const = 0;
    virtual bool                            IsHeli() const = 0;
    virtual bool                            IsPlane() const = 0;
    virtual bool                            IsBike() const = 0;
    virtual bool                            IsBmx() const = 0;
    virtual bool                            IsTrailer() const = 0;
    virtual bool                            IsVehicle() const = 0;
    virtual bool                            IsUpgrade() const = 0;

    virtual bool                            IsPed() const = 0;
    virtual bool                            IsObject() const = 0;

    virtual const char*                     GetNameIfVehicle() const = 0;

    virtual void                            Request( bool bAndLoad = false, bool bWaitForLoad = false, bool bHighPriority = false ) = 0;
    virtual void                            Remove() = 0;
    virtual unsigned char                   GetLevelFromPosition( const CVector& vecPosition ) const = 0;
    virtual bool                            IsLoaded() const = 0;
    virtual unsigned char                   GetFlags() const = 0;
    virtual const CBoundingBox*             GetBoundingBox() const = 0;
    virtual bool                            IsValid() = 0;
    virtual float                           GetDistanceFromCentreOfMassToBaseOfModel() const = 0;
    virtual unsigned short                  GetTextureDictionaryID() const = 0;
    virtual void                            SetTextureDictionaryID( unsigned short usID ) = 0;
    virtual float                           GetLODDistance() const = 0;
    virtual void                            SetLODDistance( float fDistance ) = 0;
    virtual void                            RestreamIPL() = 0;

    virtual void                            AddRef( bool bWaitForLoad, bool bHighPriority = false ) = 0;
    virtual int                             GetRefCount() const = 0;
    virtual void                            RemoveRef( bool bRemoveExtraGTARef = false ) = 0;
    virtual void                            MaybeRemoveExtraGTARef() = 0;
    virtual void                            DoRemoveExtraGTARef() = 0;

    // CVehicleModelInfo specific
    virtual short                           GetAvailableVehicleMod( unsigned short usSlot ) const = 0;
    virtual bool                            IsUpgradeAvailable( eVehicleUpgradePosn posn ) const = 0;
    virtual void                            SetCustomCarPlateText( const char *szText ) = 0;
    virtual unsigned int                    GetNumRemaps() const = 0;
    virtual void*                           GetVehicleSuspensionData() const = 0;
    virtual void*                           SetVehicleSuspensionData( void *pSuspensionLines ) = 0;

    // Upgrades only!
    virtual void                            RequestVehicleUpgrade() = 0;

    // ONLY use for peds
    virtual void                            GetVoice( short* psVoiceType, short* psVoice ) const = 0;
    virtual void                            GetVoice( const char** pszVoiceType, const char** szVoice ) const = 0;
    virtual void                            SetVoice( short sVoiceType, short sVoice ) = 0;
    virtual void                            SetVoice( const char* szVoiceType, const char* szVoice ) = 0;
};

#endif
