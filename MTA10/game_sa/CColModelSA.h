/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CColModelSA.h
*  PURPOSE:     Header file for collision model entity class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_COLMODELSA
#define __CGAMESA_COLMODELSA

#include <game/CColModel.h>

#define FUNC_CColModel_Constructor      0x40FB60
#define FUNC_CColModel_Destructor       0x40F700

typedef struct
{
    union
    {
        char version[4];
        unsigned int checksum;
    };

    DWORD size;
    char name[22];
    unsigned short modelId;
} ColModelFileHeader;

class CColFileSA
{
public:
    CColFileSA() : m_vecMin( MAX_REGION, -MAX_REGION ), m_vecMax( -MAX_REGION, MAX_REGION )
    {
        m_loaded = false;
        m_unk2 = false;
        m_isProcedural = false;
        m_isInterior = false;

        m_upperBound = 0x7FFF;
        m_lowerBound = -0;
        m_refs = 0;
    }

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    CVector2D                       m_vecMin, m_vecMax; // 0

    BYTE                            m_pad[18];          // 16

    short                           m_upperBound;       // 34
    short                           m_lowerBound;       // 36
    unsigned short                  m_refs;             // 38

    bool                            m_loaded;           // 40
    bool                            m_unk2;             // 41
    bool                            m_isProcedural;     // 42
    bool                            m_isInterior;       // 43
};

typedef struct
{
    DWORD                           pad0;               // 0
    WORD                            pad1;               // 4
    BYTE                            ucNumWheels;        // 6
    BYTE                            pad2;               // 7
    DWORD                           pad3;               // 8
    DWORD                           pad4;               // 12
    void*                           pSuspensionLines;   // 16
} CColDataSA;

class CColModelSAInterface
{
public:
                                    CColModelSAInterface();
                                    ~CColModelSAInterface();

    void                            ReleaseData();

    void*   operator new ( size_t );
    void    operator delete ( void *ptr );

    CBoundingBox                    m_bounds;           // 0
    unsigned char                   m_colPoolIndex;     // 40
    BYTE                            m_pad[3];           // 41
    CColDataSA*                     pColData;           // 44
};

class CColModelSA : public CColModel
{
    friend class CStreamingSA;
public:
                                    CColModelSA();
                                    CColModelSA( CColModelSAInterface *pInterface, bool destroy = false );
                                    ~CColModelSA();

    inline CColModelSAInterface*    GetInterface()                              { return m_pInterface; }

    bool                            Replace( unsigned short id );
    bool                            IsReplaced( unsigned short id ) const;
    bool                            Restore( unsigned short id );
    void                            RestoreAll();

    const imports_t&                GetImportList() const                       { return m_imported; }

    void                            SetOriginal( CColModelSAInterface *col )    { m_original = col; }
    CColModelSAInterface*           GetOriginal()                               { return m_original; }

private:
    void                            Apply( unsigned short id );

    CColModelSAInterface*           m_pInterface;
    CColModelSAInterface*           m_original;
    bool                            m_originalDynamic;
    bool                            m_bDestroyInterface;

    imports_t                       m_imported;
};

typedef void    (__cdecl*SetCachedCollision_t)              (unsigned int id, CColModelSAInterface *col);
extern SetCachedCollision_t     SetCachedCollision;

#endif
