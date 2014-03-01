/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.rendersetup.h
*  PURPOSE:     Entity render management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _ENTITY_RENDER_SETUP_
#define _ENTITY_RENDER_SETUP_

namespace EntityRender
{
    void                ReferenceEntityForRendering         ( CEntitySAInterface *entity );

    bool __cdecl        PushUnderwaterEntityForRendering    ( CEntitySAInterface *entity, float distance );
    int __cdecl         QueueEntityForRendering             ( CEntitySAInterface *entity, float camDistance );

    void __cdecl        ClearEntityRenderChains             ( void );
    void                ClearFallbackRenderChains           ( void );
    void __cdecl        ClearAllRenderChains                ( void );
    void __cdecl        PushEntityOnRenderQueue             ( CEntitySAInterface *entity, float camDistance );

    void                RegisterLowPriorityRenderEntity     ( CEntitySAInterface *entity );

    eRenderType __cdecl SetupEntityVisibility               ( CEntitySAInterface *entity, float& camDistance );
};

#endif //_ENTITY_RENDER_SETUP_