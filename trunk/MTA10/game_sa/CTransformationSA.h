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
    BYTE                                    m_pad[8];
    class CPlaceableSAInterface*            m_entity;

    CTransformSAInterface*                  next;
    CTransformSAInterface*                  prev;
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
                                            ~CTransformSAInterface();

    CTransformSAInterface*                  Allocate();
    bool                                    IsFreeMatrixAvailable();
    bool                                    FreeUnimportantMatrix();
    void                                    NewMatrix();

    CTransformSAInterface                   m_usedList;
    CTransformSAInterface                   m_usedItem;
    CTransformSAInterface                   m_activeList;
    CTransformSAInterface                   m_activeItem;
    CTransformSAInterface                   m_freeList;
    CTransformSAInterface                   m_freeItem;
    CTransformSAInterface*                  m_stack;
};

void Transformation_Init();

extern CTransformationSAInterface *pTransformation;

#endif //_CTransformationSA_