/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CScriptFile.h
*  PURPOSE:     Script file class header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef __CFILE_H
#define __CFILE_H

#include "CClientEntity.h"
#include <stdio.h>
#include <string>

class CScriptFile : public CClientEntity
{
    DECLARE_CLASS( CScriptFile, CClientEntity )
public:
    enum eMode
    {
        MODE_READ,
        MODE_READWRITE,
        MODE_CREATE,
    };

                            CScriptFile             ( const char *filename, size_t maxSize );
                            ~CScriptFile            ();

    // Functions required for linking
    void                    GetPosition             ( CVector& vecPosition ) const {};
    void                    SetPosition             ( const CVector& vecPosition ) {};

    // Functions required by CClientEntity
    eClientEntityType       GetType                 () const  { return SCRIPTFILE; };
    void                    Unlink                  () {};
    bool                    ReadSpecialData         ()    { return true; };

    // Load and unload routines
    bool                    Load                    ( eMode Mode );
    void                    Unload                  ();
    bool                    IsLoaded                ()    { return m_pFile != NULL; };

    bool                    IsEOF                   ();
    size_t                  GetPointer              ();
    size_t                  GetSize                 ();

    size_t                  SetPointer              ( size_t pos );
    void                    SetSize                 ( size_t size );

    void                    Flush                   ();
    size_t                  Read                    ( size_t size, char* pData );
    size_t                  Write                   ( size_t size, const char* pData );

private:
    CFile*                  m_file;
    std::string             m_filename;
    size_t                  m_maxSize;
};

#endif
