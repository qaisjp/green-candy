/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/core/CFileSystemInterface.h
*  PURPOSE:     File management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CFileSystemInterface_
#define _CFileSystemInterface_

class CFile
{
public:
    virtual                 ~CFile();

    virtual	size_t          Read( void *pBuffer, size_t sElement, long iNumElements ) = 0;
    virtual	size_t          Write( void *pBuffer, size_t sElement, long iNumElements ) = 0;
    virtual	int             Seek( long iOffset, int iType ) = 0;
    virtual	long            Tell() = 0;
    virtual	bool            IsEOF() = 0;
    virtual	bool            Stat( struct stat *pFileStats ) = 0;
    virtual	size_t          GetSize() = 0;
    virtual	void            Flush() = 0;
    virtual std::string*    GetPath() = 0;

    virtual	int             ReadInt() { return 0; }
    virtual	short           ReadShort() { return 0; }
    virtual	char            ReadByte() { return 0; }
    virtual	float           ReadFloat() { return 0; }
    virtual	size_t          WriteInt( int iInt ) { return 0; }
    virtual size_t          WriteShort( short iShort ) { return 0; }
    virtual size_t          WriteByte( char cByte ) { return 0; }
    virtual size_t          WriteFloat( float fFloat ) { return 0; }
    virtual	size_t          Printf( const char *pFormat, ... ) { return 0; }
    virtual size_t          GetString( char *pBuffer, size_t sMaxLength ) { return 0; };
};

#endif //_CFileSystemInterface_