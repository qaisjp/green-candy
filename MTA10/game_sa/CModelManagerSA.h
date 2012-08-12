/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelManagerSA.h
*  PURPOSE:     DFF model entity manager
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODEL_ENTITY_MANAGER_
#define _MODEL_ENTITY_MANAGER_

class CModelManagerSA : public CModelManager
{
    friend class CModelSA;
public:
                                CModelManagerSA();
                                ~CModelManagerSA();

    CModelSA*                   CreateModel( CFile *file, unsigned short model );

    bool                        RestoreModel( unsigned short id );
    bool                        RestoreCollision( unsigned short id );

protected:
    typedef std::vector <CModelSA*> models_t;
    models_t                m_models;
};

#endif //_MODEL_ENTITY_MANAGER_