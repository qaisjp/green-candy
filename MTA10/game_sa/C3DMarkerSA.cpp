/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/C3DMarkerSA.cpp
*  PURPOSE:     3D Marker entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void C3DMarkerSA::GetMatrix( RwMatrix& mat )
{
    mat = GetInterface()->m_mat;
}

void C3DMarkerSA::SetMatrix( const RwMatrix& mat )
{
    GetInterface()->m_mat = mat;
}

void C3DMarkerSA::SetPosition( const CVector& pos )
{
    DEBUG_TRACE("void C3DMarkerSA::SetPosition( const CVector& pos )");
    GetInterface()->m_mat.pos = pos;
}

const CVector& C3DMarkerSA::GetPosition()
{
    DEBUG_TRACE("const CVector& C3DMarkerSA::GetPosition()");
    return GetInterface()->m_mat.pos;
}

unsigned int C3DMarkerSA::GetType()
{
    DEBUG_TRACE("unsigned int C3DMarkerSA::GetType()");
    return GetInterface()->m_nType;
}

void C3DMarkerSA::SetType( unsigned int type )
{
    DEBUG_TRACE("void C3DMarkerSA::SetType( unsigned int type )");
    GetInterface()->m_nType = type;
}

bool C3DMarkerSA::IsActive()
{
    DEBUG_TRACE("BOOL C3DMarkerSA::IsActive()");
    return GetInterface()->m_bIsUsed;
}

unsigned int C3DMarkerSA::GetIdentifier()
{
    DEBUG_TRACE("DWORD C3DMarkerSA::GetIdentifier()");
    return GetInterface()->m_nIdentifier;
}

SColor C3DMarkerSA::GetColor()
{
    DEBUG_TRACE("SColor C3DMarkerSA::GetColor()");

    // From ABGR
    unsigned long ulABGR = GetInterface()->rwColour;
    SColor color;
    color.A = ( ulABGR >> 24 ) && 0xff;
    color.B = ( ulABGR >> 16 ) && 0xff;
    color.G = ( ulABGR >> 8 ) && 0xff;
    color.R = ulABGR && 0xff;
    return color;
}

void C3DMarkerSA::SetColor( const SColor color )
{
    // To ABGR
    GetInterface()->rwColour = ( color.A << 24 ) | ( color.B << 16 ) | ( color.G << 8 ) | color.R;
}

void C3DMarkerSA::SetPulsePeriod( unsigned short period )
{
    DEBUG_TRACE("void C3DMarkerSA::SetPulsePeriod( unsigned short period )");
    GetInterface()->m_nPulsePeriod = period;
}

void C3DMarkerSA::SetRotateRate( short rate )
{
    DEBUG_TRACE("void C3DMarkerSA::SetRotateRate( short rate )");
    GetInterface()->m_nRotateRate = rate;
}

float C3DMarkerSA::GetSize()
{
    DEBUG_TRACE("float C3DMarkerSA::GetSize()");
    return GetInterface()->m_fSize;
}

void C3DMarkerSA::SetSize( float size )
{
    DEBUG_TRACE("void C3DMarkerSA::SetSize( float size )");
    GetInterface()->m_fSize = size;
}

float C3DMarkerSA::GetBrightness()
{
    DEBUG_TRACE("float C3DMarkerSA::GetBrightness()");
    return GetInterface()->m_fBrightness;
}

void C3DMarkerSA::SetBrightness( float brightness )
{
    DEBUG_TRACE("void C3DMarkerSA::SetBrightness( float brightness )");
    GetInterface()->m_fBrightness = brightness;
}

void C3DMarkerSA::SetCameraRange( float range )
{
    DEBUG_TRACE("void C3DMarkerSA::SetCameraRange( float range )");
    GetInterface()->m_fCameraRange = range;
}

void C3DMarkerSA::SetPulseFraction( float fraction )
{
    DEBUG_TRACE("void C3DMarkerSA::SetPulseFraction( float fraction )");
    GetInterface()->m_fPulseFraction = fraction;
}

float C3DMarkerSA::GetPulseFraction()
{
    DEBUG_TRACE("float C3DMarkerSA::GetPulseFraction()");
    return GetInterface()->m_fPulseFraction;
}

void C3DMarkerSA::Disable()
{
    DEBUG_TRACE("void C3DMarkerSA::Disable()");
    GetInterface()->m_nIdentifier = 0;
}

void C3DMarkerSA::DeleteMarkerObject()
{
    DEBUG_TRACE("void C3DMarkerSA::DeleteMarkerObject()");

    if ( !this->GetInterface()->m_pRwObject )
        return;

    DWORD dwFunc = FUNC_DeleteMarkerObject;
    DWORD dwThis = (DWORD)GetInterface();
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    //OutputDebugString ( "Object destroyed!" );
}

void C3DMarkerSA::Reset()
{
    DEBUG_TRACE("void C3DMarkerSA::Reset()");
    m_interface->m_lastPosition = m_interface->m_mat.pos;
}
