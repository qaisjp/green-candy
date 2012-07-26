/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlaneSA.cpp
*  PURPOSE:     Plane vehicle entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPlaneSA::CPlaneSA( CPlaneSAInterface *plane ) : CAutomobileSA( plane )
{
    DEBUG_TRACE("CPlaneSA::CPlaneSA( CPlaneSAInterface *plane )");
}

CPlaneSA::~CPlaneSA()
{
}

bool CPlaneSA::IsSmokeTrailEnabled()
{
    return GetInterface()->m_smokeTrailEnabled;
}

void CPlaneSA::SetSmokeTrailEnabled( bool enabled )
{
    GetInterface()->m_smokeTrailEnabled = enabled;
}
