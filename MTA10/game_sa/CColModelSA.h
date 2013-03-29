/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CColModelSA.h
*  PURPOSE:     Header file for collision model entity class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Alberto Alonso <rydencillo@gmail.com>
*               Martin Turski <quiret@gmx.de>
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

typedef struct
{
    CVector     vecCenter;
    float       fRadius;
} CColSphereSA;

typedef struct
{
    CVector     min;
    CVector     max;
} CColBoxSA;

// Collision stored lighting
struct CColLighting
{
    uchar   day:4;    // 0-15
    uchar   night:4;  // 0-15
};

typedef struct
{
    unsigned short  v1;
    unsigned short  v2;
    unsigned short  v3;
    unsigned char   material;
    CColLighting    lighting;
} CColTriangleSA;

typedef struct
{
    BYTE pad0 [ 12 ];
} CColTrianglePlaneSA;

class CColFileSA
{
public:
    CColFileSA() : m_vecMin( MAX_REGION, -MAX_REGION ), m_vecMax( -MAX_REGION, MAX_REGION )
    {
        m_loaded = false;
        m_unk2 = false;
        m_isProcedural = false;
        m_isInterior = false;

        m_rangeStart = 0x7FFF;
        m_rangeEnd = -0;
        m_refs = 0;
    }

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    CVector2D                       m_vecMin, m_vecMax; // 0

    BYTE                            m_pad[18];          // 16

    short                           m_rangeStart;       // 34
    short                           m_rangeEnd;         // 36
    unsigned short                  m_refs;             // 38

    bool                            m_loaded;           // 40
    bool                            m_unk2;             // 41
    bool                            m_isProcedural;     // 42
    bool                            m_isInterior;       // 43
};

typedef struct
{
    unsigned short                  numSpheres;         // 0
    unsigned short                  numBoxes;           // 2
    unsigned short                  numTriangles;       // 4
    BYTE                            ucNumWheels;        // 6
    BYTE                            pad2;               // 7
    CColSphereSA*                   colSpheres;         // 8
    CColBoxSA*                      colBoxes;           // 12
    void*                           pSuspensionLines;   // 16
    void*                           unk;                // 20
    CColTriangleSA*                 colTriangles;       // 24
    CColTrianglePlaneSA*            colPlanes;          // 28
} CColDataSA;

class CColModelSAInterface
{
public:
                                    CColModelSAInterface    ( void );
                                    ~CColModelSAInterface   ( void );

    void                            ReleaseData             ( void );

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
                                    CColModelSA         ( void );
                                    CColModelSA         ( CColModelSAInterface *pInterface, bool destroy = false );
                                    ~CColModelSA        ( void );

    inline CColModelSAInterface*    GetInterface        ( void )                        { return m_pInterface; }

    bool                            Replace             ( unsigned short id );
    bool                            IsReplaced          ( unsigned short id ) const;
    bool                            Restore             ( unsigned short id );
    void                            RestoreAll          ( void );

    const imports_t&                GetImportList       ( void ) const                  { return m_imported; }

    void                            SetOriginal         ( CColModelSAInterface *col )   { m_original = col; }
    CColModelSAInterface*           GetOriginal         ( void )                        { return m_original; }

    void                            Apply               ( unsigned short id );

private:
    CColModelSAInterface*           m_pInterface;
    CColModelSAInterface*           m_original;
    bool                            m_originalDynamic;
    bool                            m_bDestroyInterface;

    imports_t                       m_imported;
};

typedef void    (__cdecl*SetCachedCollision_t)              ( unsigned int id, CColModelSAInterface *col );
extern SetCachedCollision_t     SetCachedCollision;

#endif
