/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.rtcallback.h
*  PURPOSE:     Conglomeration of all known GTA:SA render callbacks
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _ENGINE_RENDER_CALLBACKS_
#define _ENGINE_RENDER_CALLBACKS_

// temp.
#ifndef _DEBUG
#define assume( x ) __analysis_assume( (x) )
#else
#define assume( x ) assert( (x) )
#endif

namespace RenderCallbacks
{
    void        SetEnvMapRenderingEnabled( bool enabled );
    bool        IsEnvMapRenderingEnabled( void );
};

// Module initialization routines.
void RenderCallbacks_Init( void );
void RenderCallbacks_Shutdown( void );

void RenderCallbacks_Reset( void );

#endif //_ENGINE_RENDER_CALLBACKS_