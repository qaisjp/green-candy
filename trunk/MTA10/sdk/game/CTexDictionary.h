/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CTexDictionary.h
*  PURPOSE:     Texture Dictionary GTA:SA export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CTexDictionary_
#define _CTexDictionary_

class CTexDictionary
{
public:
    virtual                         ~CTexDictionary()   {};

    virtual bool                    Load( const char *filename, bool filtering ) = 0;
    virtual void                    Clear() = 0;

    virtual const char*             GetName() const = 0;
    virtual unsigned int            GetHash() const = 0;

    virtual bool                    Import( unsigned short id ) = 0;
    virtual bool                    ImportTXD( unsigned short id ) = 0;
    virtual bool                    Remove( unsigned short id ) = 0;
    virtual bool                    RemoveTXD( unsigned short id ) = 0;

    virtual bool                    IsImported( unsigned short id ) const = 0;
    virtual bool                    IsImportedTXD( unsigned short id ) const = 0;
};

#endif //_CTexDictionary_