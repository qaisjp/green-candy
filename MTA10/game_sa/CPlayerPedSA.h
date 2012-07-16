/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlayerPedSA.h
*  PURPOSE:     Header file for player ped entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               aru <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PLAYERPED
#define __CGAMESA_PLAYERPED


#include <game/CPlayerPed.h>
#include "CPedSA.h"
#include "CWantedSA.h"

#define FUNC_SetInitialState            0x60CD20
#define FUNC_ClearWeaponTarget          0x533B30

#define FUNC_CPedOperatorNew            0x5E4720 // ##SA##
#define FUNC_CPlayerPedConstructor      0x60D5B0 // ##SA##

#define FUNC_CPlayerPedDestructor       0x6093B0 // ##SA##
#define FUNC_CPlayerPedOperatorDelete   0x5E4760 // ##SA##

#define FUNC_CPlayerPed_ReApplyMoveAnims    0x609650

#define SIZEOF_CPLAYERPED               1956

class CPlayerPedSAInterface : public CPedSAInterface // CPlayerPed 1956 bytes in SA
{
public:
    void                OnFrame();
    bool                ShouldBeStealthAiming();
};

class CPlayerPedSA : public virtual CPlayerPed, public virtual CPedSA
{
private:
    bool                                m_bIsLocal;
    class CPlayerPedDataSAInterface*    m_pData;
    CWantedSA*                          m_pWanted;
    bool                                m_stealthAiming;

public:
                        CPlayerPedSA( ePedModel pedType );
                        CPlayerPedSA( CPlayerPedSAInterface * ped );
                        ~CPlayerPedSA();

    void                OnFrame();

    CWanted*            GetWanted();

    float               GetSprintEnergy();
    void                SetSprintEnergy( float fSprintEnergy );

    void                SetInitialState();

    eMoveAnim           GetMoveAnim();
    void                SetMoveAnim( eMoveAnim iAnimGroup );

    inline CPlayerPedSAInterface* GetInterface()                { return m_pInterface; };
};

#endif
