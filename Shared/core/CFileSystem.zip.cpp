/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.cpp
*  PURPOSE:     ZIP archive filesystem
*
*  Docu: https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <zlib.h>
#include <sstream>

// Include internal (private) definitions.
#include "fsinternal/CFileSystem.internal.h"
#include "fsinternal/CFileSystem.zip.internal.h"

// Global properties used by the .zip extension
// for managing temporary files (OS dependent).
// See CFileSystem::InitZIP.
static CFileTranslator *sysTmp;
static CFileTranslator *sysTmpRoot;
static unsigned int sysTmpCnt = 0;

extern CFileSystem *fileSystem;

#include "CFileSystem.Utils.hxx"

/*=======================================
    CArchiveFile

    ZIP file seeking and handling
=======================================*/

CZIPArchiveTranslator::fileDeflate::fileDeflate( CZIPArchiveTranslator& zip, file& info, CFile& sysFile ) : stream( zip, info, sysFile )
{
}

CZIPArchiveTranslator::fileDeflate::~fileDeflate( void )
{
}

size_t CZIPArchiveTranslator::fileDeflate::Read( void *buffer, size_t sElement, unsigned long iNumElements )
{
    if ( !m_readable )
        return 0;

    return m_sysFile.Read( buffer, sElement, iNumElements );
}

size_t CZIPArchiveTranslator::fileDeflate::Write( const void *buffer, size_t sElement, unsigned long iNumElements )
{
    if ( !m_writeable )
        return 0;

    m_info.UpdateTime();
    return m_sysFile.Write( buffer, sElement, iNumElements );
}

int CZIPArchiveTranslator::fileDeflate::Seek( long iOffset, int iType )
{
    return m_sysFile.Seek( iOffset, iType );
}

long CZIPArchiveTranslator::fileDeflate::Tell( void ) const
{
    return m_sysFile.Tell();
}

bool CZIPArchiveTranslator::fileDeflate::IsEOF( void ) const
{
    return m_sysFile.IsEOF();
}

bool CZIPArchiveTranslator::fileDeflate::Stat( struct stat *stats ) const
{
    tm date;

    m_info.GetModTime( date );

    date.tm_year -= 1900;

    stats->st_mtime = stats->st_atime = stats->st_ctime = mktime( &date );
    return true;
}

void CZIPArchiveTranslator::fileDeflate::PushStat( const struct stat *stats )
{
    tm *date = gmtime( &stats->st_mtime );

    m_info.SetModTime( *date );
}

void CZIPArchiveTranslator::fileDeflate::SetSeekEnd( void )
{

}

size_t CZIPArchiveTranslator::fileDeflate::GetSize( void ) const
{
    return m_sysFile.GetSize();
}

void CZIPArchiveTranslator::fileDeflate::Flush( void )
{
    m_sysFile.Flush();
}

bool CZIPArchiveTranslator::fileDeflate::IsReadable( void ) const
{
    return m_readable;
}

bool CZIPArchiveTranslator::fileDeflate::IsWriteable( void ) const
{
    return m_writeable;
}

/*=======================================
    CZIPArchiveTranslator

    ZIP translation utility
=======================================*/

CZIPArchiveTranslator::CZIPArchiveTranslator( CFile& fileStream ) : CSystemPathTranslator( false ), m_file( fileStream ), m_rootDir( filePath(), filePath() )
{
    filePath path;
    std::stringstream number_stream;

    // TODO: Get real .zip structure offset
    m_structOffset = 0;

    // Current directory starts at root
    m_curDirEntry = &m_rootDir;
    m_rootDir.parent = NULL;

    // Create temporary storage
    number_stream << sysTmpCnt++;

    std::string dirName( "/" );
    dirName += number_stream.str();
    dirName += "/";

    sysTmpRoot->CreateDir( dirName.c_str() );
    sysTmpRoot->GetFullPathFromRoot( dirName.c_str(), false, path );

    m_fileRoot = fileSystem->CreateTranslator( path.c_str() );
    m_fileRoot->CreateDir( "unpack/" );
    m_fileRoot->CreateDir( "realtime/" );

    m_unpackRoot = fileSystem->CreateTranslator( path + "unpack/" );
    m_realtimeRoot = fileSystem->CreateTranslator( path + "realtime/" );
}

CZIPArchiveTranslator::~CZIPArchiveTranslator( void )
{
    filePath path;
    m_fileRoot->GetFullPath( "@", false, path );

    delete m_unpackRoot;
    delete m_realtimeRoot;
    delete m_fileRoot;

    // Delete all temporary files
    sysTmpRoot->Delete( path.c_str() );
}

inline const CZIPArchiveTranslator::directory* CZIPArchiveTranslator::GetDeriviateDir( const directory& root, const dirTree& tree ) const
{
    const directory *curDir = &root;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end() && ( *iter == _dirBack ); ++iter )
    {
        curDir = curDir->parent;

        if ( !curDir )
            return NULL;
    }

    return GetDirTree( *curDir, iter, tree.end() );
}

inline CZIPArchiveTranslator::directory& CZIPArchiveTranslator::MakeDeriviateDir( directory& root, const dirTree& tree )
{
    directory *curDir = &root;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end() && ( *iter == _dirBack ); ++iter )
        curDir = curDir->parent;

    for ( ; iter != tree.end(); ++iter )
        curDir = &curDir->GetDirectory( *iter );

    return *curDir;
}

bool CZIPArchiveTranslator::WriteData( const char *path, const char *buffer, size_t size )
{
    // TODO
    return false;
}

bool CZIPArchiveTranslator::CreateDir( const char *path )
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( path, tree, isFile ) )
        return false;

    if ( isFile )
        tree.pop_back();

    _CreateDirTree( *m_curDirEntry, tree );
    return true;
}

CFile* CZIPArchiveTranslator::Open( const char *path, const char *mode )
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
        dir = (directory*)GetDeriviateDir( *m_curDirEntry, tree );

        if ( !dir )
            return NULL;

        entry = dir->GetFile( name );

        if ( !entry )
            return NULL;

        if ( entry->archived && !entry->cached )
        {
            dstFile = m_realtimeRoot->Open( relPath.c_str(), "wb+" );

            Extract( *dstFile, *entry );
            entry->cached = true;
        }
        else
            dstFile = m_realtimeRoot->Open( relPath.c_str(), "rb+" );

        break;
    case FILE_MODE_CREATE:
        entry = MakeDeriviateDir( *m_curDirEntry, tree ).MakeFile( name );

        if ( !entry )
            return NULL;

		// Files start off clean
		entry->Reset();

        dstFile = m_realtimeRoot->Open( relPath.c_str(), "wb+" );

        entry->cached = true;
        break;
    }

    // Seek it
    dstFile->Seek( 0, *mode == 'a' ? SEEK_END : SEEK_SET );

    fileDeflate *f = new fileDeflate( *this, *entry, *dstFile );
    f->m_path = relPath;
    f->m_writeable = ( access & FILE_ACCESS_WRITE ) != 0;
    f->m_readable = ( access & FILE_ACCESS_READ ) != 0;

    // Update the stat
    struct stat info;
    f->Stat( &info );
    dstFile->PushStat( &info );

    return f;
}

bool CZIPArchiveTranslator::Exists( const char *path ) const
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( path, tree, isFile ) )
        return false;

    if ( isFile )
    {
        filePath name = tree[ tree.size() - 1 ];
        tree.pop_back();

        const directory *dir = GetDeriviateDir( *m_curDirEntry, tree );
        return dir && dir->GetFile( name ) != NULL;
    }

    return GetDeriviateDir( *m_curDirEntry, tree ) != NULL;
}

bool CZIPArchiveTranslator::Delete( const char *path )
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( path, tree, isFile ) )
        return false;

    directory *dir;

    if ( !isFile )
    {
        if ( !( dir = (directory*)GetDeriviateDir( *m_curDirEntry, tree ) ) )
            return false;

        if ( dir->IsLocked() )
            return false;

        delete dir;

        return true;
    }

    filePath name = tree[ tree.size() - 1 ];
    tree.pop_back();

    return ( dir = (directory*)GetDeriviateDir( *m_curDirEntry, tree ) ) && dir->RemoveFile( name );
}

bool CZIPArchiveTranslator::Copy( const char *src, const char *dst )
{
    file *srcEntry = (file*)GetFileEntry( src );

    if ( !srcEntry )
        return false;

    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( dst, tree, isFile ) || !isFile )
        return false;

    filePath fileName = tree[ tree.size() - 1 ];
    tree.pop_back();

    file *dstEntry = _CreateDirTree( *m_curDirEntry, tree ).MakeFile( fileName );

    if ( !dstEntry )
        return false;

    // Copy over general attributes
    dstEntry->flags = srcEntry->flags;
    dstEntry->compression = srcEntry->compression;
    dstEntry->modTime = srcEntry->modTime;
    dstEntry->modDate = srcEntry->modDate;
    dstEntry->diskID = srcEntry->diskID;
    dstEntry->internalAttr = srcEntry->internalAttr;
    dstEntry->externalAttr = srcEntry->externalAttr;
    dstEntry->extra = srcEntry->extra;
    dstEntry->comment = srcEntry->comment;

    if ( !srcEntry->cached )
    {
        dstEntry->version = srcEntry->version;
        dstEntry->reqVersion = srcEntry->reqVersion;
        dstEntry->crc32 = srcEntry->crc32;
        dstEntry->sizeCompressed = srcEntry->sizeCompressed;
        dstEntry->sizeReal = srcEntry->sizeReal;
        dstEntry->subParsed = true;

        if ( !srcEntry->subParsed )
        {
            _localHeader header;
            seekFile( *srcEntry, header );

            CFile *dstFile = m_unpackRoot->Open( dstEntry->relPath.c_str(), "wb" );

            FileSystem::StreamCopyCount( m_file, *dstFile, header.sizeCompressed );

            delete dstFile;
        }
        else
            m_unpackRoot->Copy( srcEntry->relPath.c_str(), dstEntry->relPath.c_str() );
    }
    else
    {
        dstEntry->cached = true;

        m_realtimeRoot->Copy( srcEntry->relPath.c_str(), dstEntry->relPath.c_str() );
    }

    return true;
}

bool CZIPArchiveTranslator::Rename( const char *src, const char *dst )
{
    // TODO: add directory support.
    file *entry = (file*)GetFileEntry( src );

    if ( !entry )
        return false;

    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( dst, tree, isFile ) || !isFile )
        return false;

    filePath fileName = tree[ tree.size() - 1 ];
    tree.pop_back();

    if ( !entry->cached )
    {
        if ( !entry->subParsed )
        {
            _localHeader header;
            seekFile( *entry, header );

            CFile *dstFile = m_unpackRoot->Open( dst, "wb" );

            FileSystem::StreamCopyCount( m_file, *dstFile, header.sizeCompressed );

            delete dstFile;
        }
        else
            m_unpackRoot->Rename( entry->relPath.c_str(), dst );
    }
    else
    {
        if ( !entry->locks.empty() )
            return false;

        m_realtimeRoot->Rename( entry->relPath.c_str(), dst );
    }

    // Give it a new name
    entry->name = fileName;

    _CreateDirTree( *m_curDirEntry, tree ).MoveTo( *entry );
    return true;
}

size_t CZIPArchiveTranslator::Size( const char *path ) const
{
    const file *entry = GetFileEntry( path );

    if ( !entry )
        return 0;

    if ( !entry->cached )
        return entry->sizeReal;

    return m_realtimeRoot->Size( path );
}

bool CZIPArchiveTranslator::Stat( const char *path, struct stat *stats ) const
{
    const file *entry = GetFileEntry( path );

    if ( !entry )
        return false;

    tm date;
    entry->GetModTime( date );

    date.tm_year -= 1900;

    stats->st_mtime = stats->st_ctime = stats->st_atime = mktime( &date );
    stats->st_dev = entry->diskID;
    stats->st_rdev = 0;
    stats->st_gid = 0;
    stats->st_ino = 0;
    stats->st_mode = 0;
    stats->st_nlink = 0;
    stats->st_uid = 0;

    if ( !entry->cached )
        stats->st_size = entry->sizeReal;
    else
        stats->st_size = m_realtimeRoot->Size( entry->relPath.c_str() );

    return true;
}

bool CZIPArchiveTranslator::ReadToBuffer( const char *path, std::vector <char>& output ) const
{
    return false;
}

bool CZIPArchiveTranslator::ChangeDirectory( const char *path )
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTreeFromRoot( path, tree, isFile ) )
        return false;

    if ( isFile )
        tree.pop_back();

    directory *dir = (directory*)GetDirTree( tree );

    if ( !dir )
        return false;

    m_curDirEntry = dir;
    m_curDirTree = tree;

    m_currentDir.clear();
    _File_OutputPathTree( tree, false, m_currentDir );
    return true;
}

static void inline _ScanDirectory( const CZIPArchiveTranslator *trans, const dirTree& tree, CZIPArchiveTranslator::directory *dir, filePattern_t *pattern, bool recurse, pathCallback_t dirCallback, pathCallback_t fileCallback, void *userdata )
{
    // First scan for files.
    if ( fileCallback )
    {
        for ( CZIPArchiveTranslator::fileList::const_iterator iter = dir->files.begin(); iter != dir->files.end(); ++iter )
        {
            CZIPArchiveTranslator::file *item = *iter;

            if ( _File_MatchPattern( item->name.c_str(), pattern ) )
            {
                filePath abs_path = "/";
                _File_OutputPathTree( tree, false, abs_path );

                abs_path += item->name;

                fileCallback( abs_path, userdata );
            }
        }
    }

    // Continue with the directories
    if ( dirCallback || recurse )
    {
        for ( CZIPArchiveTranslator::directory::subDirs::const_iterator iter = dir->children.begin(); iter != dir->children.end(); ++iter )
        {
            CZIPArchiveTranslator::directory *item = *iter;

            if ( dirCallback )
            {
                filePath abs_path = "/";
                _File_OutputPathTree( tree, false, abs_path );

                abs_path += item->name;
                abs_path += "/";

                _File_OnDirectoryFound( pattern, item->name, abs_path, dirCallback, userdata );
            }

            if ( recurse )
            {
                dirTree newTree = tree;
                newTree.push_back( item->name );

                _ScanDirectory( trans, newTree, item, pattern, recurse, dirCallback, fileCallback, userdata );
            }
        }
    }
}

void CZIPArchiveTranslator::ScanDirectory( const char *dirPath, const char *wildcard, bool recurse, pathCallback_t dirCallback, pathCallback_t fileCallback, void *userdata ) const
{
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTreeFromRoot( dirPath, tree, isFile ) )
        return;

    if ( isFile )
        tree.pop_back();

    directory *dir = (directory*)GetDirTree( tree );

    if ( !dir )
        return;

    // Create a cached file pattern.
    filePattern_t *pattern = _File_CreatePattern( wildcard );

    try
    {
        _ScanDirectory( this, tree, dir, pattern, recurse, dirCallback, fileCallback, userdata );
    }
    catch( ... )
    {
        // Any exception may be thrown; we have to clean up.
        _File_DestroyPattern( pattern );
        throw;
    }

    _File_DestroyPattern( pattern );
}

static void _scanFindCallback( const filePath& path, std::vector <filePath> *output )
{
    output->push_back( path );
}

void CZIPArchiveTranslator::GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, (pathCallback_t)_scanFindCallback, NULL, &output );
}

void CZIPArchiveTranslator::GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, NULL, (pathCallback_t)_scanFindCallback, &output );
}

struct zip_mapped_rdircheck
{
    inline bool Perform( const char map[1024], size_t count, long& off ) const
    {
        unsigned short n = (unsigned int)count + 1;
        const char *cur = map + n;

        while ( n )
        {
            if ( *(unsigned int*)--cur == ZIP_SIGNATURE )
            {
                off = n + 3;
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

#pragma pack()

CZIPArchiveTranslator::directory& CZIPArchiveTranslator::_CreateDirTree( directory& dir, const dirTree& tree )
{
    directory *curDir = &dir;
    dirTree::const_iterator iter = tree.begin();

    for ( ; iter != tree.end(); ++iter )
        curDir = &curDir->GetDirectory( *iter );

    return *curDir;
}

void CZIPArchiveTranslator::ReadFiles( unsigned int count )
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

        if ( !GetRelativePathTreeFromRoot( buf, tree, isFile ) )
            throw;

        union
        {
            fsActiveEntry *fsObject;
            file *fileEntry;
            directory *dirEntry;
        };
        fsObject = NULL;

        if ( isFile )
        {
            filePath name = tree[ tree.size() - 1 ];
            tree.pop_back();

            // Deposit in the correct directory
            fileEntry = &_CreateDirTree( m_rootDir, tree ).AddFile( name );
        }
        else
        {
            // Try to create the directory.
            dirEntry = &_CreateDirTree( m_rootDir, tree );
        }

        // Initialize common data.
        if ( fsObject )
        {
            fsObject->version = header.version;
            fsObject->reqVersion = header.reqVersion;
            fsObject->flags = header.flags;
            fsObject->modTime = header.modTime;
            fsObject->modDate = header.modDate;
            fsObject->diskID = header.diskID;
            fsObject->localHeaderOffset = header.localHeaderOffset;

            if ( header.extraLen != 0 )
            {
                m_file.Read( buf, 1, header.extraLen );
                fsObject->extra = std::string( buf, header.extraLen );
            }

            if ( header.commentLen != 0 )
            {
                m_file.Read( buf, 1, header.commentLen );
                fsObject->comment = std::string( buf, header.commentLen );
            }
        }
        else
        {
            m_file.Seek( header.extraLen + header.commentLen, SEEK_CUR );
        }

        if ( fileEntry && isFile )
        {
            // Store file-orriented attributes
            fileEntry->compression = header.compression;
            fileEntry->crc32 = header.crc32;
            fileEntry->sizeCompressed = header.sizeCompressed;
            fileEntry->sizeReal = header.sizeReal;
            fileEntry->internalAttr = header.internalAttr;
            fileEntry->externalAttr = header.externalAttr;
            fileEntry->archived = true;
        }
    }
}

inline void CZIPArchiveTranslator::seekFile( const file& info, _localHeader& header )
{
    m_file.Seek( info.localHeaderOffset, SEEK_SET );

    if ( !m_file.ReadStruct( header ) )
        throw;

    if ( header.signature != ZIP_LOCAL_FILE_SIGNATURE )
        throw;

    m_file.Seek( header.nameLen + header.commentLen, SEEK_CUR );
}

struct zip_inflate_decompression
{
    zip_inflate_decompression( void )
    {
        m_stream.zalloc = NULL;
        m_stream.zfree = NULL;
        m_stream.opaque = NULL;

        if ( inflateInit2( &m_stream, -MAX_WBITS ) != Z_OK )
            throw;
    }

    ~zip_inflate_decompression( void )
    {
        inflateEnd( &m_stream );
    }

    inline void prepare( const char *buf, size_t size, bool eof )
    {
        m_stream.avail_in = size;
        m_stream.next_in = (Bytef*)buf;
    }

    inline bool parse( char *buf, size_t size, size_t& sout )
    {
        m_stream.avail_out = size;
        m_stream.next_out = (Bytef*)buf;

        switch( inflate( &m_stream, Z_NO_FLUSH ) )
        {
        case Z_DATA_ERROR:
        case Z_NEED_DICT:
        case Z_MEM_ERROR:
            throw;
        }

        sout = size - m_stream.avail_out;
        return m_stream.avail_out == 0;
    }

    z_stream m_stream;
};

void CZIPArchiveTranslator::Extract( CFile& dstFile, file& info )
{
    CFile *from;
    size_t comprSize = info.sizeCompressed;

    if ( info.subParsed )
        from = m_unpackRoot->Open( info.relPath.c_str(), "rb" );
    else
    {
        _localHeader header;
        seekFile( info, header );

        from = &m_file;
    }

    if ( info.compression == 0 )
    {
        FileSystem::StreamCopyCount( *from, dstFile, comprSize );
    }
    else if ( info.compression == 8 )
    {
        zip_inflate_decompression decompressor;

        FileSystem::StreamParserCount( *from, dstFile, comprSize, decompressor );
    }
    else
    {
        assert( 0 );
    }

    if ( info.subParsed )
        delete from;
}

CArchiveTranslator* CFileSystem::CreateZIPArchive( CFile& file )
{
    if ( !file.IsWriteable() )
        return NULL;

    return new CZIPArchiveTranslator( file );
}

CArchiveTranslator* CFileSystem::OpenArchive( CFile& file )
{
    {
        zip_mapped_rdircheck checker;

        if ( !FileSystem::MappedReaderReverse <char [1024]>( file, checker ) )
            return NULL;
    }

    _endDir dirEnd;

    if ( !file.ReadStruct( dirEnd ) )
        return NULL;

    file.Seek( -(long)dirEnd.centralDirectorySize - (long)sizeof( dirEnd ) - 4, SEEK_CUR );

    CZIPArchiveTranslator *zip = new CZIPArchiveTranslator( file );

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

void CZIPArchiveTranslator::CacheDirectory( const directory& dir )
{
    fileList::const_iterator fileIter = dir.files.begin();

    for ( ; fileIter != dir.files.end(); ++fileIter )
    {
        if ( (*fileIter)->cached || (*fileIter)->subParsed )
            continue;

        // Dump the compressed content
        _localHeader header;
        seekFile( **fileIter, header );

        CFile *dst = m_unpackRoot->Open( (*fileIter)->relPath.c_str(), "wb" );

        FileSystem::StreamCopyCount( m_file, *dst, (*fileIter)->sizeCompressed );

        delete dst;

        (*fileIter)->subParsed = true;
    }

    directory::subDirs::const_iterator iter = dir.children.begin();

    for ( ; iter != dir.children.end(); ++iter )
        CacheDirectory( **iter );
}

struct zip_stream_compression
{
    zip_stream_compression( CZIPArchiveTranslator::_localHeader& header ) : m_header( header )
    {
        m_header.sizeCompressed = 0;
    }

    inline void checksum( const char *buf, size_t size )
    {
        m_header.crc32 = crc32( m_header.crc32, (Bytef*)buf, size );
    }

    inline void prepare( const char *buf, size_t size, bool eof )
    {
        m_rcv = size;
        m_buf = buf;

        checksum( buf, size );
    }

    inline bool parse( char *buf, size_t size, size_t& sout )
    {
        size_t toWrite = std::min( size, m_rcv );
        memcpy( buf, m_buf, toWrite );

        m_header.sizeCompressed += toWrite;

        m_buf += toWrite;
        m_rcv -= toWrite;

        sout = size;
        return m_rcv != 0;
    }

    size_t m_rcv;
    const char* m_buf;
    CZIPArchiveTranslator::_localHeader& m_header;
};

struct zip_deflate_compression : public zip_stream_compression
{
    zip_deflate_compression( CZIPArchiveTranslator::_localHeader& header, int level ) : zip_stream_compression( header )
    {
        m_stream.zalloc = NULL;
        m_stream.zfree = NULL;
        m_stream.opaque = NULL;

        if ( deflateInit2( &m_stream, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY ) != Z_OK )
            throw;
    }

    ~zip_deflate_compression( void )
    {
        deflateEnd( &m_stream );
    }

    inline void prepare( const char *buf, size_t size, bool eof )
    {
        m_flush = eof ? Z_FINISH : Z_NO_FLUSH;

        m_stream.avail_in = size;
        m_stream.next_in = (Bytef*)buf;

        zip_stream_compression::checksum( buf, size );
    }

    inline bool parse( char *buf, size_t size, size_t& sout )
    {
        m_stream.avail_out = size;
        m_stream.next_out = (Bytef*)buf;

        int ret = deflate( &m_stream, m_flush );

        sout = size - m_stream.avail_out;

        m_header.sizeCompressed += sout;

        if ( ret == Z_STREAM_ERROR )
            throw;

        return m_stream.avail_out == 0;
    }

    int m_flush;
    z_stream m_stream;
};

void CZIPArchiveTranslator::SaveDirectory( directory& dir, size_t& size )
{
    if ( dir.NeedsWriting() )
    {
        _localHeader header = dir.ConstructLocalHeader();
        
        // Allocate space in the archive.
        dir.AllocateArchiveSpace( this, header, size );

        header.version = dir.version;
        header.flags = dir.flags;
        header.compression = 0;
        header.crc32 = 0;
        header.sizeCompressed = 0;
        header.sizeReal = 0;

        m_file.WriteStruct( header );
        m_file.WriteString( dir.relPath );
        m_file.WriteString( dir.comment );
    }
    
    directory::subDirs::iterator iter = dir.children.begin();

    for ( ; iter != dir.children.end(); ++iter )
        SaveDirectory( **iter, size );

    fileList::iterator fileIter = dir.files.begin();

    for ( ; fileIter != dir.files.end(); ++fileIter )
    {
        file& info = **fileIter;

        _localHeader header = info.ConstructLocalHeader();

        // Allocate space in the archive.
        info.AllocateArchiveSpace( this, header, size );

        if ( !info.cached )
        {
            header.version = info.version;
            header.flags = info.flags;
            header.compression = info.compression;
            header.crc32 = info.crc32;
            header.sizeCompressed = info.sizeCompressed;
            header.sizeReal = info.sizeReal;

            size += info.sizeCompressed;

            m_file.WriteStruct( header );
            m_file.WriteString( info.relPath.c_str() );
            m_file.WriteString( info.comment );

            CFile *src = m_unpackRoot->Open( info.relPath.c_str(), "rb" );

            FileSystem::StreamCopy( *src, m_file );

            delete src;
        }
        else    // has to be cached
        {
            header.version = 10;    // WINNT
            header.flags = info.flags;
            header.compression = info.compression = 8; // deflate
            header.crc32 = 0;

            m_file.WriteStruct( header );
            m_file.WriteString( info.relPath.c_str() );
            m_file.WriteString( info.comment );

            CFile *src = dynamic_cast <CSystemFileTranslator*> ( m_realtimeRoot )->OpenEx( info.relPath.c_str(), "rb", FILE_FLAG_WRITESHARE );

            info.sizeReal = header.sizeReal = src->GetSize();

            if ( header.compression == 0 )
            {
                zip_stream_compression compressor( header );

                FileSystem::StreamParser( *src, m_file, compressor );
            }
            else if ( header.compression == 8 )
            {
                zip_deflate_compression compressor( header, Z_DEFAULT_COMPRESSION );

                FileSystem::StreamParser( *src, m_file, compressor );
            }
            else
            {
                assert( 0 );
            }

            delete src;

            size += info.sizeCompressed = header.sizeCompressed;
            info.crc32 = header.crc32;

            long wayOff = header.sizeCompressed + header.nameLen + header.commentLen;

            m_file.Seek( -wayOff - (long)sizeof( header ), SEEK_CUR );
            m_file.WriteStruct( header );
            m_file.Seek( wayOff, SEEK_CUR );
        }
    }
}

unsigned int CZIPArchiveTranslator::BuildCentralFileHeaders( const directory& dir, size_t& size )
{
    unsigned cnt = 0;

    if ( dir.NeedsWriting() )
    {
        _centralFile header = dir.ConstructCentralHeader();

        // Zero out fields which make no sense
        header.compression = 0;
        header.crc32 = 0;
        header.sizeCompressed = 0;
        header.sizeReal = 0;
        header.internalAttr = 0;
#ifdef _WIN32
        header.externalAttr = FILE_ATTRIBUTE_DIRECTORY;
#else
        header.externalAttr = 0;
#endif
        
        m_file.WriteStruct( header );
        m_file.WriteString( dir.relPath );
        m_file.WriteString( dir.extra );
        m_file.WriteString( dir.comment );
        
        cnt++;
    }

    directory::subDirs::const_iterator iter = dir.children.begin();

    for ( ; iter != dir.children.end(); ++iter )
        cnt += BuildCentralFileHeaders( **iter, size );

    fileList::const_iterator fileIter = dir.files.begin();

    for ( ; fileIter != dir.files.end(); ++fileIter )
    {
        const file& info = **fileIter;
        _centralFile header = info.ConstructCentralHeader();

        header.compression = info.compression;
        header.crc32 = info.crc32;
        header.sizeCompressed = info.sizeCompressed;
        header.sizeReal = info.sizeReal;
        header.internalAttr = info.internalAttr;
        header.externalAttr = info.externalAttr;

        m_file.WriteStruct( header );
        m_file.WriteString( info.relPath.c_str() );
        m_file.WriteString( info.extra );
        m_file.WriteString( info.comment );

        size += sizeof( header ) + header.nameLen + header.extraLen + header.commentLen;
        cnt++;
    }

    return cnt;
}

void CZIPArchiveTranslator::Save( void )
{
    if ( !m_file.IsWriteable() )
        return;

    // Cache the .zip content
    CacheDirectory( m_rootDir );

    // Rewrite the archive
    m_file.Seek( m_structOffset, SEEK_SET );

    size_t fileSize = 0;
    SaveDirectory( m_rootDir, fileSize );

    // Create the central directory
    size_t centralSize = 0;
    unsigned int entryCount = BuildCentralFileHeaders( m_rootDir, centralSize );

    // Finishing entry
    m_file.WriteInt( ZIP_SIGNATURE );

    _endDir tail;
    tail.diskID = 0;
    tail.diskAlign = 0;
    tail.entries = entryCount;
    tail.totalEntries = entryCount;
    tail.centralDirectorySize = centralSize;
    tail.centralDirectoryOffset = fileSize; // we might need the offset of the .zip here
    tail.commentLen = m_comment.size();

    m_file.WriteStruct( tail );
    m_file.WriteString( m_comment );

    // Cap the stream
    m_file.SetSeekEnd();
}

bool _File_CreateDirectory( const char *osPath )
{
#ifdef __linux__
    if ( mkdir( osPath, FILE_ACCESS_FLAG ) == 0 )
        return true;

    switch( errno )
    {
    case EEXIST:
    case 0:
        return true;
    }

    return false;
#elif defined(_WIN32)
    return CreateDirectory( osPath, NULL ) != FALSE;
#else
    return false;
#endif //OS DEPENDANT CODE
}

void CFileSystem::InitZIP( void )
{
    filePath tmpDir;

#ifdef _WIN32
    char buf[1024];

    GetTempPath( sizeof( buf ), buf );
    tmpDir = buf;
    tmpDir += '/';
#elif defined(__linux__)
    const char *dir = getenv("TEMPDIR");

    if ( !dir )
        tmpDir = "/tmp";
    else
        tmpDir = dir;

    tmpDir += '/';

    // On linux we cannot be sure that our directory exists.
    if ( !_File_CreateDirectory( tmpDir.c_str() ) )
        exit( 7098 );
#endif //OS DEPENDANT CODE

    sysTmp = CreateTranslator( tmpDir.c_str() );

    std::stringstream stream;

    stream.precision( 0 );
    stream << ( rand() % 647251833 );

    tmpDir += "&$!reAr";
    tmpDir += stream.str();
    tmpDir += "_/";

    // Make sure the temporary directory exists.
    _File_CreateDirectory( tmpDir.c_str() );

    // Create the .zip temporary root
    sysTmpRoot = CreateTranslator( tmpDir.c_str() );
}

void CFileSystem::DestroyZIP( void )
{
    filePath tmpDir;
    sysTmpRoot->GetFullPath( "@", false, tmpDir );

    delete sysTmpRoot;

    sysTmp->Delete( tmpDir.c_str() );

    delete sysTmp;
}
