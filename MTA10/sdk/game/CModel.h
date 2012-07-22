/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CModel.h
*  PURPOSE:     RenderWare Model entity export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MODEL_ENTITY_EXPORT_
#define _MODEL_ENTITY_EXPORT_

class CModel
{
public:
    virtual                         ~CModel()   {}

    virtual const char*             GetName() const = 0;
    virtual unsigned int            GetHash() const = 0;

    virtual std::vector <unsigned short>    GetImportList() const = 0;

    virtual bool                    Replace( unsigned short id ) = 0;
    virtual bool                    IsReplaced( unsigned short id ) const = 0;
    virtual bool                    Restore( unsigned short id ) = 0;

    virtual void                    RestoreAll() = 0;
};

#endif //_MODEL_ENTITY_EXPORT_