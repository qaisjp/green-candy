/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedModelInfoSA.cpp
*  PURPOSE:     Modelinfo for ped entities
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

CPedModelInfoSAInterface::CPedModelInfoSAInterface ( void )
{
    m_pColModel = (CColModelSAInterface*)VAR_CTempColModels_ModelPed1;

    Init();
}

CPedModelInfoSAInterface::~CPedModelInfoSAInterface()
{

}

eModelType CPedModelInfoSAInterface::GetModelType()
{
    return MODEL_PED;
}

void CPedModelInfoSAInterface::DeleteRwObject()
{
    CClumpModelInfoSAInterface::DeleteRwObject();

    if (m_pColModel)
    {
        delete m_pColModel;

        m_pColModel = NULL;
    }
}

void CPedModelInfoSAInterface::SetAnimFile( const char *name )
{

}

void CPedModelInfoSAInterface::ConvertAnimFileIndex()
{

}

int CPedModelInfoSAInterface::GetAnimFileIndex()
{
    return -1;
}

CPedModelInfoSA::CPedModelInfoSA() : CModelInfoSA ()
{
    m_pPedModelInterface = new CPedModelInfoSAInterface;
}

CPedModelInfoSA::~CPedModelInfoSA()
{

}

void CPedModelInfoSA::SetMotionAnimGroup ( AssocGroupId animGroup )
{
    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = (DWORD)FUNC_SetMotionAnimGroup;
    _asm
    {
        mov     ecx, dwThis
        push    animGroup
        call    dwFunc
    }
}
