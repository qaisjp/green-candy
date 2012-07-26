/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTrailerSA.cpp
*  PURPOSE:     Trailer vehicle entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CTrailerSA::CTrailerSA( CTrailerSAInterface *trailer ) : CAutomobileSA( trailer )
{
    DEBUG_TRACE("CTrailerSA::CTrailerSA( CTrailerSAInterface *trailer )");
}