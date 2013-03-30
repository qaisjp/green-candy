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
    inline CModelLoadInfoSA&    GetModelLoadInfo( unsigned short id )
    {
        return *( (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id );
    }

    CBaseModelInfoSAInterface* __cdecl  GetModelByHash      ( unsigned int hash, unsigned short *id );
    CBaseModelInfoSAInterface*          GetModelInfoByName  ( const char *name, unsigned short startId, unsigned short endId, unsigned short *id );
};

// IPL sector (buildings) instance
class CIPLFileSA
{
public:
    CIPLFileSA( void ) : m_vecMin( MAX_REGION, -MAX_REGION ), m_vecMax( -MAX_REGION, MAX_REGION )
    {
        m_unk1 = HEIGHT_BOUND;
        m_unk2 = -HEIGHT_BOUND;

        m_unk3 = HEIGHT_BOUND;
        m_unk4 = -HEIGHT_BOUND;

        m_unk5 = 0xFFFF;

        m_unk6 = m_unk7 = m_unk8 = false;
        m_unk9 = true;

        m_unk10 = m_unk11 = false;
    }

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    CVector2D       m_vecMin, m_vecMax;     // 0
    char            m_name[18];             // 16

    short           m_unk1;                 // 34
    short           m_unk2;                 // 36

    short           m_unk3;                 // 38
    short           m_unk4;                 // 40

    unsigned short  m_unk5;                 // 42

    bool            m_unk6;                 // 44
    bool            m_unk7;                 // 45
    bool            m_unk8;                 // 46
    bool            m_unk9;                 // 47, 1
    bool            m_unk10;                // 48
    bool            m_unk11;                // 49

    WORD            m_pad;                  // 50
};

void StreamingLoader_Init( void );
void StreamingLoader_Shutdown( void );

#endif //_STREAMING_INIT_