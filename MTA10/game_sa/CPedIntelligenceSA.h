/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedIntelligenceSA.h
*  PURPOSE:     Header file for ped entity AI class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PEDINTELLIGENCE
#define __CGAMESA_PEDINTELLIGENCE

#include <game/CPedIntelligence.h>
#include "CEventSA.h"
class CTaskManagerSA;
class CVehicleScannerSA;

#include "CTaskManagerSA.h"
#include "CVehicleScannerSA.h"

#define FUNC_IsRespondingToEvent                    0x600DB0
#define FUNC_GetCurrentEvent                        0x4ABE70
#define FUNC_GetCurrentEventType                    0x4ABE60
#define FUNC_CPedIntelligence_TestForStealthKill    0x601E00

class CPed;

class CFightManagerSAInterface
{
public:
    BYTE            m_pad1 [ 16 ];
    BYTE            m_unknownState;
    BYTE            m_pad2 [ 3 ];
    float           m_strafeState;
    float           m_forwardBackwardState;
};

class CPedIntelligenceSAInterface 
{
public:
    void*   operator new( size_t );
    void    operator delete( void *ptr );

// CEventHandlerHistory @ + 56
    CPedSAInterface*                m_ped;
    CTaskManagerSAInterface*        m_taskManager;          // 4
    BYTE                            m_pad[16];              // 8
    CFightManagerSAInterface*       m_fightInterface;       // 24
    BYTE                            m_pad2[184];            // 28
    CVehicleScannerSAInterface*     m_vehicleScanner;       // 212

    BYTE                            m_pad3[444];            // 216
};

class CPedIntelligenceSA : public CPedIntelligence
{
public:
                                    CPedIntelligenceSA ( CPedIntelligenceSAInterface *intelligence, CPed *ped );
                                    ~CPedIntelligenceSA ( void );

    CPedIntelligenceSAInterface*    GetInterface ( void ) { return m_interface; }

    bool                            IsRespondingToEvent ( void );
    int                             GetCurrentEventType ( void );
    CEvent*                         GetCurrentEvent ( void );

    CTaskManager*                   GetTaskManager( void );
    CVehicleScanner*                GetVehicleScanner( void );

    bool                            TestForStealthKill ( CPed * pPed, bool bUnk );

private:
    CPedIntelligenceSAInterface*    m_internalInterface;
    CPedSA*                         m_ped;
    CTaskManagerSA*                 m_taskManager;
    CVehicleScannerSA*              m_vehicleScanner;
};

#endif
