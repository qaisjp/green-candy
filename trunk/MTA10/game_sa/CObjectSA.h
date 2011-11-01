/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CObjectSA.h
*  PURPOSE:     Header file for object entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "CPedSA.h"

#ifndef __CGAMESA_OBJECT
#define __CGAMESA_OBJECT

#include <game/CObject.h>
#include "CPhysicalSA.h"


#define FUNC_CObject_Create             0x5A1F60
#define FUNC_CObject_Explode            0x5A1340

class CObjectSAInterface : public CPhysicalSAInterface // + 372 = burn time stop , +348 = scale // +340 = health
{
public:
    void*   operator new( size_t );
    void    operator delete( void *ptr );

    BYTE                        m_pad[100];
};

class CObjectSA : public virtual CObject, public virtual CPhysicalSA
{
private:
    unsigned char               m_ucAlpha;
    bool                        m_bIsAGangTag;

public:
                                CObjectSA           ( CObjectSAInterface * objectInterface );
                                CObjectSA           ( DWORD dwModel );
                                ~CObjectSA          ( void );

    inline CObjectSAInterface * GetObjectInterface  ( void )    { return ( CObjectSAInterface * ) GetInterface (); }

    void                        Explode             ( void );
    void                        Break               ( void );
    void                        SetScale            ( float fScale );
    void                        SetHealth           ( float fHealth );
    float                       GetHealth           ( void );
    void                        SetModelIndex       ( unsigned short ulModel );

    inline void                 SetAlpha            ( unsigned char ucAlpha ) { m_ucAlpha = ucAlpha; }
    inline unsigned char        GetAlpha            ( ) { return m_ucAlpha; }

    bool                        IsAGangTag          ( ) const { return m_bIsAGangTag; }
private:
    void                        CheckForGangTag     ( );
};

/*
#define COBJECTSA_DEFINED
#define COBJECTSAINTERFACE_DEFINED
*/

#endif
