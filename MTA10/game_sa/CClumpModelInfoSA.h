/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CClumpModelInfoSA.h
*  PURPOSE:     Clump model instance
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CClumpModelInfoSA_
#define _CClumpModelInfoSA_

#define MAX_BONES   64

#define ATOMIC_HIER_ACTIVE          0x00000001
#define ATOMIC_HIER_FRONTSEAT       0x00000008
#define ATOMIC_HIER_DOOR            0x00000010
#define ATOMIC_HIER_SEAT            0x00000200
#define ATOMIC_HIER_UNKNOWN4        0x00010000
#define ATOMIC_HIER_UNKNOWN3        0x00020000
#define ATOMIC_HIER_UNKNOWN5        0x00100000
#define ATOMIC_HIER_UNKNOWN6        0x00200000

class CAtomicHierarchySAInterface
{
public:
    const char*                     m_name;
    unsigned int                    m_frameHierarchy;
    unsigned int                    m_flags;
};

class CClumpModelInfoSAInterface : public CBaseModelInfoSAInterface
{
public:
    virtual CColModelSAInterface* __thiscall        GetCollision() = 0;
    virtual void __thiscall                         SetClump( RpClump *clump ) = 0;

    void                    Init();
    void                    Shutdown();
    void                    DeleteRwObject();
    eRwType                 GetRwModelType();
    RpClump*                CreateRwObjectEx( int rwTag );
    RpClump*                CreateRwObject();
    void                    SetAnimFile( const char *name );
    void                    ConvertAnimFileIndex();
    int                     GetAnimFileIndex();

    void                    AssignAtomics( CAtomicHierarchySAInterface *atomics );

    RpClump*                m_rwClump;                      // 28
    int                     m_animBlock;                    // 32
};

#endif //_CClumpModelInfoSA_