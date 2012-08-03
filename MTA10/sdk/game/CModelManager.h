/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CModelManager.h
*  PURPOSE:     RenderWare Model entity export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODEL_MANAGER_EXPORT_
#define _MODEL_MANAGER_EXPORT_

class CModelManager
{
public:
    virtual CModel*                     CreateModel( const char *path, unsigned short model ) = 0;

    virtual bool                        RestoreModel( unsigned short id ) = 0;
};

#endif //_MODEL_MANAGER_EXPORT_