/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare/CTexDictionary.h
*  PURPOSE:     Texture Dictionary GTA:SA export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTexDictionary_
#define _CTexDictionary_

class CTexDictionary abstract : public virtual CRwObject
{
public:
    virtual                         ~CTexDictionary()   {};

    virtual std::list <CTexture*>&  GetTextures() = 0;
    virtual void                    Clear() = 0;

    virtual void                    SetGlobalEmitter() = 0;

    virtual bool                    Import( unsigned short id ) = 0;
    virtual bool                    ImportTXD( unsigned short id ) = 0;
    virtual bool                    Remove( unsigned short id ) = 0;
    virtual bool                    RemoveTXD( unsigned short id ) = 0;

    virtual void                    ClearImports() = 0;

    virtual bool                    IsImported( unsigned short id ) const = 0;
    virtual bool                    IsImportedTXD( unsigned short id ) const = 0;

    virtual bool                    IsUsed() const = 0;

    typedef std::list <unsigned short> importList_t;

    virtual const importList_t&     GetImportedList() const = 0;
};

#endif //_CTexDictionary_