/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare/CRpAtomic.h
*  PURPOSE:     RenderWare RpAtomic modification export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_ATOMIC_
#define _RW_ATOMIC_

class CModel;

class CRpAtomic abstract : public virtual CRwObject
{
public:
    virtual                         ~CRpAtomic()    {}

    virtual CRpAtomic*              Clone() const = 0;

    virtual void                    Render() = 0;

    virtual void                    GetWorldSphere( RwSphere& out ) = 0;

    virtual void                    AddToModel( CModel *model ) = 0;
    virtual CModel*                 GetModel() = 0;
    virtual void                    RemoveFromModel() = 0;

    virtual bool                    Replace( unsigned short id ) = 0;
    virtual bool                    IsReplaced( unsigned short id ) const = 0;
    virtual bool                    Restore( unsigned short id ) = 0;
    virtual void                    RestoreAll() = 0;

    typedef std::vector <unsigned short> imports_t;

    virtual const imports_t&        GetImportList() const = 0;
};

#endif //_RW_ATOMIC_