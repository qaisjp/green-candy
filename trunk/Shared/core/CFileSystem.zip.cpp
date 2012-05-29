/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.cpp
*  PURPOSE:     ZIP archive filesystem
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <zlib.h>

#define ZIP_SIGNATURE               0x06054B50
#define ZIP_FILE_SIGNATURE          0x02014B50
#define ZIP_LOCAL_FILE_SIGNATURE    0x04034B50

static CFileTranslator *sysTmp;
static CFileTranslator *sysTmpRoot;
static unsigned int sysTmpCnt = 0;

extern CFileSystem *fileSystem;

#include "CFileSystem.Utils.hxx"

/*=======================================
    CArchiveFile

    ZIP file seeking and handling
=======================================*/

CArchiveFileTranslator::fileDeflate::fileDeflate( CArchiveFileTranslator& zip, file& info, CFile& sysFile ) : stream( zip, info, sysFile )
{
}

CArchiveFileTranslator::fileDeflate::~fileDeflate()
{
}

size_t CArchiveFileTranslator::fileDeflate::Read( void *buffer, size_t sElement, unsigned long iNumElements )
{
    if ( !m_readable )
        return 0;

    return m_sysFile.Read( buffer, sElement, iNumElements );
}

size_t CArchiveFileTranslator::fileDeflate::Write( const void *buffer, size_t sElement, unsigned long iNumElements )
{
    if ( !m_writeable )
        return 0;

    return m_sysFile.Write( buffer, sElement, iNumElements );
}

int CArchiveFileTranslator::fileDeflate::Seek( long iOffset, int iType )
{
    return m_sysFile.Seek( iOffset, iType );
}

long CArchiveFileTranslator::fileDeflate::Tell() const
{
    return m_sysFile.Tell();
}

bool CArchiveFileTranslator::fileDeflate::IsEOF() const
{
    return m_sysFile.IsEOF();
}

bool CArchiveFileTranslator::fileDeflate::Stat( struct stat *stats ) const
{
    tm date;

    m_info.GetModTime( date );

    date.tm_year -= 1900;

    stats->st_mtime = mktime( &date );
    stats->st_atime = stats->st_mtime;
    stats->st_ctime = stats->st_mtime;
    return true;
}

void CArchiveFileTranslator::fileDeflate::PushStat( const struct stat *stats )
{
    tm *date = gmtime( &stats->st_mtime );
    
    m_info.SetModTime( *date );
}

size_t CArchiveFileTranslator::fileDeflate::GetSize() const
{
    return m_sysFile.GetSize();
}

void CArchiveFileTranslator::fileDeflate::Flush()
{
    m_sysFile.Flush();
}

bool CArchiveFileTranslator::fileDeflate::IsReadable() const
{
    return m_readable;
}

bool CArchiveFileTranslator::fileDeflate::IsWriteable() const
{
    return m_writeable;
}

/*=======================================
    CArchiveFileTranslator

    ZIP translation utility
=======================================*/

CArchiveFileTranslator::CArchiveFileTranslator( CFile& file ) : m_file( file )
{
    filePath path;
    std::stringstream stream;

    // Current directory starts at root
    m_curDirEntry = &m_root;
    m_root.parent = NULL;

    // Create temporary storage
    stream << sysTmpCnt++;

    std::string dirName( "/" );
    dirName += stream.str();
    dirName += "/"; 

    sysTmpRoot->CreateDir( dirName.c_str() );
    sysTmpRoot->GetFullPathFromRoot( dirName.c_str(), false, path );

    m_fileRoot = (CSystemFileTranslator*)fileSystem->CreateTranslator( path.c_str() );
}

CArchiveFileTranslator::~CArchiveFileTranslator()
{
    filePath path;
    m_fileRoot->GetFullPath( "/", false, path );

    delete m_fileRoot;

    // Delete all temporary files
    sysTmpRoot->Delete( path.c_str() );
}

inline CArchiveFileTranslator::directory* CArchiveFileTranslator::GetDeriviateDir( directory& root, const dirTree& tree )
{
    directory *curDir = &root;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end() && ( *iter == _dirBack ); iter++ )
    {
        curDir = curDir->parent;

        if ( !curDir )
            return NULL;
    }

    return GetDirTree( *curDir, iter, tree.end() );
}

inline CArchiveFileTranslator::directory& CArchiveFileTranslator::MakeDeriviateDir( directory& root, const dirTree& tree )
{
    directory *curDir = &root;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end() && ( *iter == _dirBack ); iter++ )
        curDir = curDir->parent;

    for ( ; iter != tree.end(); iter++ )
        curDir = &curDir->GetDirectory( *iter );

    return *curDir;
}

bool CArchiveFileTranslator::WriteData( const char *path, const char *buffer, size_t size )
{
    return false;
}

bool CArchiveFileTranslator::CreateDir( const char *path )
{
    dirTree tree;
    bool file;

    if ( !GetRelativePathTree( path, tree, file ) )
        return false;

    if ( file )
        tree.pop_back();

    _CreateDirTree( *m_curDirEntry, tree );
    return true;
}

#define ZIP_FILE_MODE_CREATE    0x01
#define ZIP_FILE_MODE_OPEN      0x02

#define ZIP_ACCESS_WRITE        0x01
#define ZIP_ACCESS_READ         0x02

CFile* CArchiveFileTranslator::Open( const char *path, const char *mode )
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( path, tree, isFile ) || !isFile )
        return NULL;

    filePath name = tree[ tree.size() - 1 ];
    tree.pop_back();

    unsigned int m;
    unsigned int access;

    if ( !_File_ParseMode( *this, path, mode, access, m ) )
        return NULL;

    directory *dir;
    file *entry;
    CFile *dstFile;

    // We require the offset path, too
    filePath relPath;
    GetRelativePathFromRoot( path, true, relPath );

    switch( m )
    {
    case FILE_MODE_OPEN:
        dir = GetDeriviateDir( *m_curDirEntry, tree );

        if ( !dir )
            return NULL;

        entry = dir->GetFile( name );

        if ( !entry )
            return NULL;

        if ( entry->archived && !entry->cached )
        {
            dstFile = m_fileRoot->Open( relPath.c_str(), "wb+" );

            Extract( *dstFile, *entry );
            entry->cached = true;
        }
        else
            dstFile = m_fileRoot->Open( relPath.c_str(), "rb+" );

        break;
    case FILE_MODE_CREATE:
        dir = &MakeDeriviateDir( *m_curDirEntry, tree );
        entry = dir->GetFile( name );

        if ( entry )
            dir->RemoveFile( *entry );

        entry = &dir->AddFile( name );

        dstFile = m_fileRoot->Open( relPath.c_str(), "wb+" );
        break;
    }

    // Seek it
    dstFile->Seek( 0, *mode == 'a' ? SEEK_END : SEEK_SET );

    fileDeflate *f = new fileDeflate( *this, *entry, *dstFile );
    f->m_path = relPath;
    f->m_writeable = ( access & FILE_ACCESS_READ ) != 0;
    f->m_readable = ( access & FILE_ACCESS_WRITE ) != 0;

    // Update the stat
    struct stat info;
    f->Stat( &info );
    dstFile->PushStat( &info );

    return f;
}

bool CArchiveFileTranslator::Exists( const char *path ) const
{
    return false;
}

bool CArchiveFileTranslator::Delete( const char *path )
{
    return false;
}

bool CArchiveFileTranslator::Copy( const char *src, const char *dst )
{
    return false;
}

bool CArchiveFileTranslator::Rename( const char *src, const char *dst )
{
    return false;
}

size_t CArchiveFileTranslator::Size( const char *path ) const
{
    return 0;
}

bool CArchiveFileTranslator::Stat( const char *path, struct stat *stats ) const
{
    return false;
}

bool CArchiveFileTranslator::ReadToBuffer( const char *path, std::vector <char>& output ) const
{
    return false;
}

bool CArchiveFileTranslator::ChangeDirectory( const char *path )
{
    dirTree tree;
    bool file;

    if ( !GetRelativePathTreeFromRoot( path, tree, file ) )
        return false;

    if ( file )
        tree.pop_back();

    directory *dir = GetDirTree( tree );

    if ( !dir )
        return false;

    m_curDirEntry = dir;
    m_curDirTree = tree;

    m_currentDir.clear();
    _File_OutputPathTree( tree, false, m_currentDir );
    return true;
}

void CArchiveFileTranslator::ScanDirectory( const char *directory, const char *wildcard, bool recurse, pathCallback_t dirCallback, pathCallback_t fileCallback, void *userdata ) const
{
    
}

static void _scanFindCallback( const filePath& path, std::vector <filePath> *output )
{
    output->push_back( path );
}

void CArchiveFileTranslator::GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, (pathCallback_t)_scanFindCallback, NULL, &output );
}

void CArchiveFileTranslator::GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, NULL, (pathCallback_t)_scanFindCallback, &output );
}

struct zip_mapped_rdircheck
{
    inline unsigned short operator() ( const char map[1024], long& off ) const
    {
        unsigned short n = 1021;
        const char *cur = map + 1021;

        while ( n )
        {
            if ( *(unsigned int*)cur-- == ZIP_SIGNATURE )
            {
                off = n + 4;
                return true;
            }

            n--;
        }

        return false;
    }
};

#pragma pack(1)

struct _endDir
{
    unsigned short  diskID;
    unsigned short  diskAlign;
    unsigned short  entries;
    unsigned short  totalEntries;
    size_t          centralDirectorySize;
    size_t          centralDirectoryOffset;

    unsigned short  commentLen;
};

struct _centralFile
{
    unsigned int    signature;
    unsigned short  version;
    unsigned short  reqVersion;
    unsigned short  flags;
    unsigned short  compression;
    unsigned short  modTime;
    unsigned short  modDate;
    unsigned int    crc32;

    size_t          sizeCompressed;
    size_t          sizeReal;

    unsigned short  nameLen;
    unsigned short  extraLen;
    unsigned short  commentLen;

    unsigned short  diskID;
    unsigned short  internalAttr;
    unsigned int    externalAttr;
    size_t          localHeaderOffset;
};

struct _localHeader
{
    unsigned int    signature;
    unsigned short  version;
    unsigned short  flags;
    unsigned short  compression;
    unsigned short  modTime;
    unsigned short  modDate;
    unsigned int    crc32;

    size_t          sizeCompressed;
    size_t          sizeReal;
    
    unsigned short  nameLen;
    unsigned short  commentLen;
};

#pragma pack()

CArchiveFileTranslator::directory& CArchiveFileTranslator::_CreateDirTree( directory& dir, const dirTree& tree )
{
    directory *curDir = &dir;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end(); iter++ )
        curDir = &curDir->GetDirectory( *iter );

    return *curDir;
}

void CArchiveFileTranslator::ReadFiles( unsigned int count )
{
    char buf[65536];

    while ( count-- )
    {
        _centralFile header;

        if ( !m_file.ReadStruct( header ) || header.signature != ZIP_FILE_SIGNATURE )
            throw;

        // Acquire the path
        m_file.Read( buf, 1, header.nameLen );
        buf[ header.nameLen ] = 0;

        dirTree tree;
        bool isFile;

        if ( !GetRelativePathTreeFromRoot( buf, tree, isFile ) || !isFile )
            throw;

        filePath name = tree[ tree.size() - 1 ];
        tree.pop_back();

        // Deposit in the correct directory
        file& entry = _CreateDirTree( m_root, tree ).AddFile( name );

        // Store attributes
        entry.version = header.version;
        entry.reqVersion = header.reqVersion;
        entry.flags = header.flags;
        entry.compression = header.compression;
        entry.modTime = header.modTime;
        entry.modDate = header.modDate;
        entry.crc32 = header.crc32;
        entry.sizeCompressed = header.sizeCompressed;
        entry.sizeReal = header.sizeReal;
        entry.diskID = header.diskID;
        entry.internalAttr = header.internalAttr;
        entry.externalAttr = header.externalAttr;
        entry.localHeaderOffset = header.localHeaderOffset;
        entry.archived = true;
        entry.cached = false;

        if ( header.extraLen )
        {
            m_file.Read( buf, 1, header.extraLen );
            entry.extra = std::string( buf, header.extraLen );
        }

        if ( header.commentLen )
        {
            m_file.Read( buf, 1, header.commentLen );
            entry.comment = std::string( buf, header.commentLen );
        }
    }
}

void CArchiveFileTranslator::Extract( CFile& dstFile, file& info )
{
    m_file.Seek( info.localHeaderOffset, SEEK_SET );

    _localHeader header;
    size_t s = sizeof( header );

    if ( !m_file.ReadStruct( header ) )
        throw;

    if ( header.signature != ZIP_LOCAL_FILE_SIGNATURE )
        throw;

    m_file.Seek( header.nameLen + header.commentLen, SEEK_CUR );

    char buf[16384];

    switch( header.compression )
    {
    case 0:
        {
            size_t toRead;

            while ( ( toRead = min( sizeof( buf ), header.sizeCompressed ) ) != 0 )
            {
                size_t rb = m_file.Read( buf, 1, toRead );

                header.sizeCompressed -= rb;

                dstFile.Write( buf, 1, rb );
            }
        }
        break;
    case 8:
        {
            int ret;
            char outbuf[16384];
            z_stream stream;
            stream.zalloc = NULL;
            stream.zfree = NULL;
            stream.opaque = NULL;
            stream.avail_in = 0;
            stream.next_in = NULL;

            inflateInit2( &stream, -MAX_WBITS );

            do
            {
                size_t toRead = min( sizeof( buf ), header.sizeCompressed );

                if ( toRead == 0 )
                    break;

                size_t rb = m_file.Read( buf, 1, toRead );
                header.sizeCompressed -= rb;

                stream.avail_in = rb;
                stream.next_in = (Bytef*)buf;

                do
                {
                    stream.avail_out = sizeof( outbuf );
                    stream.next_out = (Bytef*)outbuf;

                    ret = inflate( &stream, Z_NO_FLUSH );

                    dstFile.Write( outbuf, 1, sizeof( outbuf ) - stream.avail_out );

                } while ( stream.avail_out == 0 );

            } while ( ret != Z_STREAM_END );

            inflateEnd( &stream );
        }
        break;
    default:
        assert( 0 );
    }
}

CFileTranslator* CFileSystem::CreateZIPArchive( CFile& file )
{
    if ( !file.IsWriteable() )
        return NULL;

    return new CArchiveFileTranslator( file );
}

CFileTranslator* CFileSystem::OpenArchive( CFile& file )
{
    size_t size = file.GetSize();

    if ( !FileSystem::MappedReaderReverse <char [1024]>( file, zip_mapped_rdircheck() ) )
        return NULL;

    _endDir dirEnd;

    if ( !file.ReadStruct( dirEnd ) )
        return NULL;

    file.Seek( -(long)dirEnd.centralDirectorySize - (long)sizeof( dirEnd ) - 4, SEEK_CUR );

    CArchiveFileTranslator *zip = new CArchiveFileTranslator( file );

    try
    {
        zip->ReadFiles( dirEnd.entries );
    }
    catch( ... )
    {
        delete zip;

        return NULL;
    }

    return zip;
}

void CArchiveFileTranslator::Save()
{
    
}

void CFileSystem::InitZIP()
{
    char buf[1024];
    filePath tmpDir;

#ifdef _WIN32
    GetTempPath( sizeof( buf ), buf );
    tmpDir = buf;
    tmpDir += '/';

    sysTmp = CreateTranslator( tmpDir.c_str() );

    std::stringstream stream;

    stream.precision( 0 );
    stream << ( rand() % 647251833 );

    tmpDir += "&$!reAr";
    tmpDir += stream.str();
    tmpDir += "_/";

    CreateDirectory( tmpDir.c_str(), NULL );
#endif //_WIN32

    // Create the .zip temporary root
    sysTmpRoot = CreateTranslator( tmpDir.c_str() );
}

void CFileSystem::DestroyZIP()
{
    filePath tmpDir;
    sysTmpRoot->GetFullPath( "/", false, tmpDir );

    delete sysTmpRoot;

    sysTmp->Delete( tmpDir.c_str() );

    delete sysTmp;
}