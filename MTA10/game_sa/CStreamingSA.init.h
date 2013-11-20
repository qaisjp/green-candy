/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.init.h
*  PURPOSE:     Header file for streaming initialization routines
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMING_INIT_
#define _STREAMING_INIT_

// IMG archive entries stored in a global array (parallel to model info array)
#define ARRAY_CModelLoadInfo                                0x008E4CC0

#define VAR_StreamerThreadHandle                            0x008E4008

class CBaseModelInfoSAInterface;

// Streaming exports
namespace Streaming
{
    inline CModelLoadInfoSA&    GetModelLoadInfo( modelId_t id )
    {
        return *( (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id );
    }

    inline CModelLoadInfoSA&    GetModelLoadInfo( modelId_t offset, modelId_t id )
    {
        return *( ( (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + offset ) + id );
    }

    inline CModelLoadInfoSA*    GetNextLoadInfo( CModelLoadInfoSA *info )
    {
        if ( info->m_primaryModel == 0xFFFF )
            return NULL;

        return &GetModelLoadInfo( info->m_primaryModel );
    }

    inline CModelLoadInfoSA*    GetPrevLoadInfo( CModelLoadInfoSA *info )
    {
        if ( info->m_secondaryModel == 0xFFFF )
            return NULL;

        return &GetModelLoadInfo( info->m_secondaryModel );
    }

    inline CModelLoadInfoSA*    GetQueuedLoadInfo( void )
    {
        return GetNextLoadInfo( *(CModelLoadInfoSA**)0x008E4C58 );
    }

    inline CModelLoadInfoSA*    GetLastQueuedLoadInfo( void )
    {
        return GetPrevLoadInfo( *(CModelLoadInfoSA**)0x008E4C54 );
    }

    inline CModelLoadInfoSA*    GetGarbageCollectModel( void )
    {
        return GetPrevLoadInfo( *(CModelLoadInfoSA**)0x008E4C5C );
    }

    inline CModelLoadInfoSA*    GetLastGarbageCollectModel( void )
    {
        return *(CModelLoadInfoSA**)0x008E4C60;
    }

    CBaseModelInfoSAInterface* __cdecl  GetModelByHash      ( unsigned int hash, modelId_t *id );
    CBaseModelInfoSAInterface* __cdecl  GetModelByName      ( const char *name, modelId_t *id );
    CBaseModelInfoSAInterface*          GetModelInfoByName  ( const char *name, modelId_t startId, modelId_t endId, modelId_t *id );

    unsigned int __cdecl RegisterCollision( const char *name );
    unsigned int __cdecl RegisterIPLFile( const char *name );
};

void StreamingLoader_Init( void );
void StreamingLoader_Shutdown( void );

#endif //_STREAMING_INIT_