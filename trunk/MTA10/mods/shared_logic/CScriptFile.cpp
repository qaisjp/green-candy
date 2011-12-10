/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CScriptFile.cpp
*  PURPOSE:     Script file class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

CScriptFile::CScriptFile( const char *filename, size_t maxSize ) : ClassInit ( this ), CClientEntity ( INVALID_ELEMENT_ID )
{
    // Init
    SetTypeName( "file" );

    m_file = NULL;
    m_filename = filename ? filename : "";
    m_maxSize = maxSize;
}

CScriptFile::~CScriptFile()
{
    // Close the file
    Unload();
}

bool CScriptFile::Load( eMode Mode )
{
    // If we haven't already got a file
    if ( m_file )
        return false;

    switch( Mode )
    {
    case MODE_READ:
        m_file = resFileRoot->Open( m_strFilename.c_str(), "rb" );
        break;

    case MODE_READWRITE:
        m_file = resFileRoot->Open( m_strFilename.c_str(), "rb+" );
        break;

    case MODE_CREATE:
        m_file = resFileRoot->Open( m_strFilename.c_str(), "wb+" );
        break;
    }

    // Return whether we successfully opened it or not
    return m_file != NULL;
}

void CScriptFile::Unload()
{
    if ( !m_file )
        return;

    // Close the file
    delete m_file;

    m_file = NULL;
}

bool CScriptFile::IsEOF()
{
    if ( !m_file )
        return true;

    // Reached end of file?
    return m_file->IsEOF();
}

size_t CScriptFile::GetPointer()
{
    if ( !m_file )
        return 0;

    return m_file->Tell();
}

size_t CScriptFile::GetSize()
{
    if ( !m_file )
        return 0;

    return m_file->GetSize();
}

size_t CScriptFile::SetPointer( size_t pos )
{
    if ( !m_file )
        return 0;

    // No idea what that is...
    if ( pos > m_file->GetSize() )
        pos = max( pos, m_maxSize );

    // Move the pointer
    m_file->Seek( (long)pos, SEEK_SET );

    pos = max( pos, m_file->GetSize() );

    // Return the new position
    return pos;
}

void CScriptFile::SetSize( size_t size )
{
    if ( !m_file )
        return;

    m_file->SetSize( size );
}

void CScriptFile::Flush()
{
    if ( !m_file )
        return;

    m_file->Flush();
}

size_t CScriptFile::Read( size_t size, char *data )
{
    if ( !m_file )
        return 0;

    // Try to read data into the given block. Return number of bytes we read.
    return m_file->Read( data, 1, size );
}

size_t CScriptFile::Write( size_t size, const char *data )
{
    if ( !m_file )
        return 0;

    // Write the data into the given block. Return number of bytes we wrote.
    return m_file->Write( data, 1, size );
}
