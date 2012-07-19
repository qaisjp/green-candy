/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCameraSA.cpp
*  PURPOSE:     Camera rendering
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
/**
 * \todo Need to add other TakeControl functions
 */

static bool bCameraClipObjects;
static bool bCameraClipVehicles;

#define VAR_CameraClipVehicles              0x8A5B14
#define VAR_CameraClipDynamicObjects        0x8A5B15
#define VAR_CameraClipStaticObjects         0x8A5B16

#define HOOKPOS_Camera_CollisionDetection   0x520190
DWORD RETURN_Camera_CollisionDetection =    0x520195;
void HOOK_Camera_CollisionDetection ();

CCameraSA::CCameraSA( CCameraSAInterface *cam )
{ 
    DEBUG_TRACE("CCameraSA::CCameraSA(CCameraSAInterface * cameraInterface)");

    m_interface = cam;

    for( unsigned int i = 0; i<MAX_CAMS; i++ )
        m_cams[i] = new CCamSA( &m_interface->m_cams[i] );

    bCameraClipObjects = true;
    bCameraClipVehicles = true;

    HookInstall( HOOKPOS_Camera_CollisionDetection, (DWORD)HOOK_Camera_CollisionDetection, 5 );
}

CCameraSA::~CCameraSA()
{
    for ( unsigned int i = 0; i < MAX_CAMS; i++ )
        delete m_cams[i];
}

void CCameraSA::Restore()
{
    DEBUG_TRACE("void CCameraSA::Restore()");

    DWORD dwFunc = FUNC_Restore;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
    }
}

void CCameraSA::RestoreWithJumpCut()
{
    DEBUG_TRACE("void CCameraSA::RestoreWithJumpCut()");

    CCameraSAInterface * cameraInterface = this->GetInterface();
    DWORD dwFunc = 0x50BD40;
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
    }
    dwFunc = 0x50BAB0;
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
    }
}

/**
 * \todo Find out what the last two paramters are
 */
void CCameraSA::TakeControl( CEntity *entity, eCamMode CamMode, int switchStyle )
{
    DEBUG_TRACE("VOID CCameraSA::TakeControl(CEntity * entity, eCamMode CamMode, int CamSwitchStyle)");

    CEntitySA *pEntitySA = dynamic_cast <CEntitySA*> ( entity );

    if ( !pEntitySA )
        return;

    CEntitySAInterface *itent = pEntitySA->GetInterface();
    CCameraSAInterface *cmint = GetInterface();
    // __thiscall

    DWORD CCamera__TakeControl = FUNC_TakeControl;
    _asm 
    {
        mov ecx, cmint
        push 1
        push CamSwitchStyle
        push CamMode
        push itent
        call CCamera__TakeControl
    }
}

void CCameraSA::TakeControl( CVector& pos, int switchStyle )
{
    DEBUG_TRACE("VOID CCameraSA::TakeControl(CVector * position, int CamSwitchStyle)");
    CCameraSAInterface *cmint = GetInterface();
    // __thiscall
    CVector vecOffset;
/*  vecOffset.fZ = 0.5f;
    vecOffset.fY = 0.5f;
    vecOffset.fX = 0.5f;*/
/*  DWORD dwFunc = 0x50BEC0;
    _asm
    {
        mov ecx, cameraInterface
        lea     eax, vecOffset
        push    eax
        push    position
        call    dwFunc
    }*/

    DWORD CCamera__TakeControlNoEntity = FUNC_TakeControlNoEntity;
    _asm 
    {
        mov ecx, cmint
        push 1
        push CamSwitchStyle
        push position
        call CCamera__TakeControlNoEntity
    }

    DWORD dwFunc = 0x50BEC0;
    _asm
    {
        mov ecx, cmintss
        lea     eax, vecOffset
        push    eax
        push    position
        call    dwFunc
    }

}

// vecOffset: an offset from 0,0,0
// vecLookAt: an offset from 0,0,0

void CCameraSA::TakeControlAttachToEntity( CEntity *target, CEntity *attach, CVector& offset, CVector& lookAt, float tilt, int switchStyle )
{
    DEBUG_TRACE("void CCameraSA::TakeControlAttachToEntity( CEntity *target, CEntity *attach, CVector& offset, CVector& lookAt, float tilt, int switchStyle )");

    char szDebug[255] = {'\0'};
    CEntitySAInterface *teint = 0;
    CEntitySAInterface *aeint = 0;

    if( target )
    {
        CEntitySA* pTargetEntitySA = dynamic_cast <CEntitySA*> ( target );
        if ( pTargetEntitySA )
            teint = pTargetEntitySA->GetInterface();
    }

    CEntitySA* pAttachEntitySA = dynamic_cast <CEntitySA*> ( attach );

    if ( !pAttachEntitySA )
        return;

    aeint = pAttachEntitySA->GetInterface();

    CCameraSAInterface *cmint = GetInterface();
    // __thiscall

    //  void    TakeControlAttachToEntity(CEntity* NewTarget, CEntity* AttachEntity, 
    //  CVector& vecOffset, CVector& vecLookAt, float fTilt, short CamSwitchStyle, 
    //  int WhoIsTakingControl=SCRIPT_CAM_CONTROL);
    DWORD CCamera__TakeControlAttachToEntity = FUNC_TakeControlAttachToEntity;

    _asm 
    {
        mov ecx,cmint
        push 1
        push switchStyle
        push tilt
        push lookAt
        push offset
        push aeint
        push teint
        call CCamera__TakeControlAttachToEntity
    }
}

const RwMatrix& CCameraSA::GetMatrix()
{
    return GetInterface()->m_cameraMatrix; // ->Placeable.matrix;
}

void CCameraSA::SetMatrix( const RwMatrix& mat )
{
    mat = GetInterface()->m_cameraMatrix;
}

VOID CCameraSA::SetCamPositionForFixedMode ( CVector * vecPosition, CVector * vecUpOffset )
{
    DWORD dwFunc = FUNC_SetCamPositionForFixedMode;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    vecUpOffset
        push    vecPosition
        call    dwFunc
    }
}

VOID CCameraSA::Find3rdPersonCamTargetVector ( FLOAT fDistance, CVector * vecGunMuzzle, CVector * vecSource, CVector * vecTarget )
{
    DEBUG_TRACE("VOID CCameraSA::Find3rdPersonCamTargetVector ( FLOAT fDistance, CVector * vecGunMuzzle, CVector * vecSource, CVector * vecTarget )");
    FLOAT fOriginX = vecGunMuzzle->fX;
    FLOAT fOriginY = vecGunMuzzle->fY;
    FLOAT fOriginZ = vecGunMuzzle->fZ;
    DWORD dwFunc = FUNC_Find3rdPersonCamTargetVector;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    vecTarget
        push    vecSource
        push    fOriginZ
        push    fOriginY
        push    fOriginX
        push    fDistance
        call    dwFunc
    }
}

float CCameraSA::Find3rdPersonQuickAimPitch ( void )
{
    DEBUG_TRACE("float CCameraSA::Find3rdPersonQuickAimPitch ( void )");
    float fReturn;
    DWORD dwFunc = FUNC_Find3rdPersonQuickAimPitch;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
        fstp    fReturn
    }
    return fReturn;
}

BYTE CCameraSA::GetActiveCam()
{
    DEBUG_TRACE("BYTE CCameraSA::GetActiveCam()");
    CCameraSAInterface * cameraInterface = this->GetInterface();
    return cameraInterface->ActiveCam;
}

CCamSA* CCameraSA::GetCam(BYTE bCameraID)
{
    DEBUG_TRACE("CCam * CCameraSA::GetCam(BYTE bCameraID)");
    
    if(bCameraID<MAX_CAMS)
        return Cams[bCameraID];

    return NULL;
}

CCamSA* CCameraSA::GetCam( CCamSAInterface* camInterface )
{
    for ( unsigned int i = 0; i < MAX_CAMS; i++ )
    {
        if ( m_cams[i] && m_cams[i]->GetInterface() == camInterface )
        {
            return m_cams [i];
        }
    }

    return NULL;
}


VOID CCameraSA::SetWidescreen(BOOL bWidescreen)
{
    DEBUG_TRACE("VOID CCameraSA::SetWidescreen(BOOL bWidescreen)");
    CCameraSAInterface * cameraInterface = this->GetInterface();
    if(bWidescreen == FALSE)
        cameraInterface->m_WideScreenOn = false;
    else
        cameraInterface->m_WideScreenOn = true;
}

BOOL CCameraSA::GetWidescreen()
{
    DEBUG_TRACE("BOOL CCameraSA::GetWidescreen()");
    CCameraSAInterface * cameraInterface = this->GetInterface();
    return cameraInterface->m_WideScreenOn;
}
/**
 * \todo Rewrite these functions to use m_nCarZoom presumeably
 */
float CCameraSA::GetCarZoom()
{
    DEBUG_TRACE("float CCameraSA::GetCarZoom()");
/*  CCameraSAInterface * cameraInterface = this->GetInterface();
    char szDebug[255] = {'\0'};
    sprintf(szDebug, "%d", (DWORD)&cameraInterface->CarZoomIndicator - (DWORD)cameraInterface);
    OutputDebugString(szDebug);
    return cameraInterface->CarZoomIndicator;   */
    return NULL;
}
/**
 * \todo Check if this ever works?
 */
VOID CCameraSA::SetCarZoom(float fCarZoom)
{
    DEBUG_TRACE("VOID CCameraSA::SetCarZoom(float fCarZoom)");
    /*CCameraSAInterface * cameraInterface = this->GetInterface();
    cameraInterface->CarZoomIndicator = fCarZoom;   */
}

/**
 * \todo does this need an enum?
 * This eventually calls TakeControl if its a valid cam mode at that time
 */
bool CCameraSA::TryToStartNewCamMode(DWORD dwCamMode)
{
    DEBUG_TRACE("VOID CCameraSA::TryToStartNewCamMode(DWORD dwCamMode)");
    DWORD dwFunc = FUNC_TryToStartNewCamMode;
    bool bReturn = false;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    dwCamMode
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

bool CCameraSA::ConeCastCollisionResolve(CVector *pPos, CVector *pLookAt, CVector *pDest, float rad, float minDist, float *pDist)
{
    DWORD dwFunc = FUNC_ConeCastCollisionResolve;
    bool bReturn = false;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    pDist
        push    minDist
        push    rad
        push    pDest
        push    pLookAt
        push    pPos
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

void CCameraSA::VectorTrackLinear ( CVector * pTo, CVector * pFrom, float time, bool bSmoothEnds )
{
    DWORD dwFunc = FUNC_VectorTrackLinear;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    bSmoothEnds
        push    time
        push    pFrom
        push    pTo
        call    dwFunc
    }
}

bool CCameraSA::IsFading ( void )
{
    DWORD dwFunc = FUNC_GetFading;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    bool bRet = false;
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
        mov     bRet, al
    }
    return bRet;
}

int CCameraSA::GetFadingDirection ( void )
{
    DWORD dwFunc = FUNC_GetFadingDirection;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    int dwRet = false;
    _asm
    {
        mov     ecx, cameraInterface
        call    dwFunc
        mov     dwRet, eax
    }
    return dwRet;
}

void CCameraSA::Fade ( float fFadeOutTime, int iOutOrIn )
{
    DWORD dwFunc = FUNC_Fade;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    _asm
    {
        mov     ecx, cameraInterface
        push    iOutOrIn
        push    fFadeOutTime
        call    dwFunc
    }
}

void CCameraSA::SetFadeColor ( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue )
{
    DWORD dwFunc = FUNC_SetFadeColour;
    CCameraSAInterface * cameraInterface = this->GetInterface();
    DWORD dwRed = ucRed;
    DWORD dwGreen = ucGreen;
    DWORD dwBlue = ucBlue;
    _asm
    {
        mov     ecx, cameraInterface
        push    dwBlue
        push    dwGreen
        push    dwRed
        call    dwFunc
    }
}

float CCameraSA::GetCameraRotation ( void )
{
    return *(float *)VAR_CameraRotation;
}

RwMatrix* CCameraSA::GetLTM ( void )
{
    return internalInterface->m_pRwCamera->m_parent->m_ltm;
}

CEntity* CCameraSA::GetTargetEntity ( void )
{
    CEntitySAInterface* pInterface = GetInterface()->pTargetEntity;
    CPoolsSA* pPools = ((CPoolsSA *)pGame->GetPools());

    switch( pInterface->nType )
    {
    case ENTITY_TYPE_PED:
        return pPools->GetPed( pInterface );
    case ENTITY_TYPE_VEHICLE:
        return pPools->GetVehicle( pInterface );
    case ENTITY_TYPE_OBJECT:
        return pPools->GetObject( pInterface );
    }
    return NULL;
}

void CCameraSA::SetCameraClip ( bool bObjects, bool bVehicles )
{
    bCameraClipObjects = bObjects;
    bCameraClipVehicles = bVehicles;
}


void _cdecl DoCameraCollisionDetectionPokes ()
{
    if ( !bCameraClipObjects )
    {
        MemPutFast < char > ( VAR_CameraClipDynamicObjects, 0 );
        MemPutFast < char > ( VAR_CameraClipStaticObjects, 0 );
    }
    else
        MemPutFast < char > ( VAR_CameraClipStaticObjects, 1 );

    if ( !bCameraClipVehicles )
        MemPutFast < char > ( VAR_CameraClipVehicles, 0 );
}

void _declspec(naked) HOOK_Camera_CollisionDetection ()
{
    _asm
    {
        pushad
        call DoCameraCollisionDetectionPokes
        popad
        sub         esp,24h
        push        ebx 
        push        ebp
        jmp         RETURN_Camera_CollisionDetection
    }
}

BYTE CCameraSA::GetCameraViewMode ( void )
{
    // TODO: Add support for ped camera view, this will only work on vehicles for now.
    return *(BYTE *)VAR_VehicleCameraView;
}

VOID CCameraSA::SetCameraViewMode ( BYTE dwCamMode )
{
    MemPut < BYTE > ( VAR_VehicleCameraView, dwCamMode );
}
