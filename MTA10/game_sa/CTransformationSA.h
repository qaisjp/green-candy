/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTransformationSA.cpp
*  PURPOSE:     Transformation matrix management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTransformationSA_
#define _CTransformationSA_

#define CLASS_CTransformation       0x00B74288
#define FUNC_InitTransformation     0x0054F3A0

class CTransformSAInterface : public RwMatrix
{
public:
                            CTransformSAInterface();
                            ~CTransformSAInterface();

    void operator =( const RwMatrix& mat )
    {
        assign( mat );
    }

    CTransformSAInterface*                  m_unk;          // 64, I do not know what these are
    CTransformSAInterface*                  m_unk2;         // 68
    union
    {
        class CPlaceableSAInterface*        m_entity;       // 72
        float                               m_unkFloat;     // 72
    };

    CTransformSAInterface*                  next;           // 76
    CTransformSAInterface*                  prev;           // 80
};

class CAllocatedTransformSAInterface
{
public:
    inline CTransformSAInterface*           Get( unsigned int index )
    {
        if ( index >= m_count )
            return NULL;

        return (CTransformSAInterface*)(this + 1) + index;
    }

    unsigned int                            m_count;
};

class CTransformationSAInterface
{
public:
                                            CTransformationSAInterface( unsigned int max );
                                            ~CTransformationSAInterface();

    CTransformSAInterface*                  Allocate();
    bool                                    IsFreeMatrixAvailable();
    bool                                    FreeUnimportantMatrix();
    void                                    NewMatrix();

    CTransformSAInterface                   m_usedList;         // 0
    CTransformSAInterface                   m_usedItem;         // 84
    CTransformSAInterface                   m_activeList;       // 168
    CTransformSAInterface                   m_activeItem;       // 252
    CTransformSAInterface                   m_freeList;         // 336
    CTransformSAInterface                   m_freeItem;         // 420
    CTransformSAInterface*                  m_stack;            // 504
};

void Transformation_Init();

extern CTransformationSAInterface *const pTransform;

#endif //_CTransformationSA_