/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamerSA.h
*  PURPOSE:     GTA:SA entity streamer extensions
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMER_EXTENSIONS_
#define _STREAMER_EXTENSIONS_

// Streamer variables.
// The streaming sectors are a*a quads, hence rows == cols always true.
#define ARRAY_StreamSectors                                 0x00B7D0B8
#define NUM_StreamSectorRows                                120
#define NUM_StreamSectorCols                                120
#define ARRAY_StreamRepeatSectors                           0x00B992B8
#define NUM_StreamRepeatSectorRows                          16
#define NUM_StreamRepeatSectorCols                          16
#define ARRAY_StreamUpdateSectors                           0x00B99EB8
#define NUM_StreamUpdateSectorRows                          30
#define NUM_StreamUpdateSectorCols                          30

// Global game API for streaming entities
namespace Streamer
{
    typedef CPtrNodeSingleSA <CEntitySAInterface> entityLink_t;

    // Structures used to save streamed-in entities in.
    typedef ptrNodeSingleContainer <CEntitySAInterface> streamSectorEntry;

    template <typename callbackType>
    inline void ExecuteChain( streamSectorEntry& sector, callbackType& cb )
    {
        if ( Streamer::entityLink_t *ptrNode = sector.GetList() )
        {
            cb.EntryCallback( sector.m_list );

            while ( ptrNode && ptrNode->m_next )
            {
                cb.EntryCallback( ptrNode->m_next );

                ptrNode = ptrNode->m_next;
            }
        }
    }

    struct streamPrimarySectorEntry
    {
        streamSectorEntry       first;      // 0
        streamSectorEntry       second;     // 4
    };

    struct streamRepeatSectorEntry
    {
        streamSectorEntry       first;      // 0
        streamSectorEntry       second;     // 4
        streamSectorEntry       third;      // 8
    };

    // Helper function to scan all entities that are streamed in.
    template <typename callbackType>
    void __forceinline ForAllStreamedEntities( callbackType& cb, bool fast )
    {
        // Scan all first-class entities.
        for ( unsigned int n = 0; n < NUM_StreamSectorRows * NUM_StreamSectorCols; n++ )
        {
            streamPrimarySectorEntry *entry = ((streamPrimarySectorEntry*)ARRAY_StreamSectors + n);

            // First the primary entry.
            ExecuteChain( entry->first, cb );

            // Next the second entry.
            ExecuteChain( entry->second, cb );
        }

        // Now for the second-class entities.
        // ccw has asked whether they are big buildings/he indicated toward it.
        for ( unsigned int n = 0; n < NUM_StreamRepeatSectorRows * NUM_StreamRepeatSectorCols; n++ )
        {
            streamRepeatSectorEntry *entry = (streamRepeatSectorEntry*)ARRAY_StreamRepeatSectors + n;

            // We have three streaming entries here.
            if ( !fast )
            {
                // Here comes the first.
                ExecuteChain( entry->first, cb );
            }

            // Now for the second.
            ExecuteChain( entry->second, cb );

            if ( !fast )
            {
                // Finally the third.
                ExecuteChain( entry->third, cb );
            }
        }
    }

    // API for utilities.
    void __cdecl ResetScanIndices( void );
    void __cdecl RequestStaticEntitiesOfSector( const streamSectorEntry& entryPtr, unsigned int reqFlags );
    void __cdecl RequestStaticEntities(
        const streamSectorEntry& entryPtr, float x, float y,
        float minX, float minY, float maxX, float maxY,
        float camFarClip, unsigned int reqFlags
    );
    void __cdecl RequestSquaredSectorEntities( const CVector& reqPos, unsigned int reqFlags );
    void __cdecl RequestAdvancedSquaredSectorEntities( const CVector& reqPos, unsigned int reqFlags );
};

// Module management routines.
void    Streamer_Init( void );
void    Streamer_Shutdown( void );

#endif //_STREAMER_EXTENSIONS_