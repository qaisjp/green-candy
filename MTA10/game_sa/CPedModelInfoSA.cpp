/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedModelInfoSA.cpp
*  PURPOSE:     Modelinfo for ped entities
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

CPedModelInfoSAInterface::CPedModelInfoSAInterface ( void )
{
    *(DWORD*)this = VAR_CPedModelInfo_VTBL;
    m_pColModel = ( CColModelSAInterface * ) VAR_CTempColModels_ModelPed1;

    Init();
}

CPedModelInfoSAInterface::~CPedModelInfoSAInterface()
{

}

CAtomicModelInfoSA* CPedModelInfoSAInterface::GetAtomicModelInfo()
{
    return NULL;
}

CDamageAtomicModelInfoSA* CPedModelInfoSAInterface::GetDamageAtomicModelInfo()
{
    return NULL;
}

CLODAtomicModelInfoSA* CPedModelInfoSAInterface::GetLODAtomicModelInfo()
{
    return NULL;
}

eModelType CPedModelInfoSAInterface::GetModelType()
{
    return MODEL_PED;
}

unsigned int CPedModelInfoSAInterface::GetTimeInfo()
{
    return 0;
}

void CPedModelInfoSAInterface::Init()
{
    CClumpModelInfoSAInterface::Init();

    m_flags = 0xFFFFFFFF;
}

void CPedModelInfoSAInterface::Shutdown()
{
    CClumpModelInfoSAInterface::Shutdown();
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

bool CPedModelInfoSAInterface::SetAnimFile( const char *name )
{
    return false;
}

void CPedModelInfoSAInterface::ConvertAnimFileIndex()
{

}

int CPedModelInfoSAInterface::GetAnimFileIndex()
{
    return -1;
}

CPedModelInfoSA::CPedModelInfoSA ( void ) : CModelInfoSA ()
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
