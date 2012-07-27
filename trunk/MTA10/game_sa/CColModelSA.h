/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CColModelSA.h
*  PURPOSE:     Header file for collision model entity class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
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
    char version[4];
    DWORD size;
    char name[0x18];
} ColModelFileHeader;

typedef struct
{
    DWORD                           pad0;
    WORD                            pad1;
    BYTE                            ucNumWheels;
    BYTE                            pad2;
    DWORD                           pad3;
    DWORD                           pad4;
    void*                           pSuspensionLines;
} CColDataSA;

class CColModelSAInterface
{
public:
                            ~CColModelSAInterface();

    void*   operator new( size_t );
    void    operator delete( void *ptr );

    CBoundingBox                    m_bounds;
    BYTE                            m_pad[4];
    CColDataSA*                     pColData;
};

class CColModelSA : public CColModel
{
public:
                                    CColModelSA     ( void );
                                    CColModelSA     ( CColModelSAInterface * pInterface );
                                    ~CColModelSA    ( void );

    inline CColModelSAInterface *   GetInterface    ( void ) { return m_pInterface; }
    inline void                     Destroy         ( void ) { delete this; }

private:
    CColModelSAInterface *          m_pInterface;
    bool                            m_bDestroyInterface;
};

#endif
