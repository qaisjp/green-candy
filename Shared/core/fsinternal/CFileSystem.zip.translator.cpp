/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.translator.cpp
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

// Global properties used by the .zip extension
static unsigned int sysTmpCnt = 0;

// Include internal (private) definitions.
#include "fsinternal/CFileSystem.internal.h"
#include "fsinternal/CFileSystem.zip.internal.h"

extern CFileSystem *fileSystem;

#include "CFileSystem.Utils.hxx"
#include "CFileSystem.zip.utils.hxx"

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

CZIPArchiveTranslator::CZIPArchiveTranslator( zipExtension& theExtension, CFile& fileStream ) : CSystemPathTranslator( false ), m_zipExtension( theExtension ), m_file( fileStream ), m_rootDir( filePath(), filePath() )
{
    // TODO: Get real .zip structure offset
    m_structOffset = 0;

    // Current directory starts at root
    m_curDirEntry = &m_rootDir;
    m_rootDir.parent = NULL;

    m_fileRoot = NULL;
    m_realtimeRoot = NULL;
    m_unpackRoot = NULL;
}

CZIPArchiveTranslator::~CZIPArchiveTranslator( void )
{
    filePath path;

    bool needDeletion = ( m_fileRoot != NULL );

    if ( m_fileRoot )
    {
        m_fileRoot->GetFullPath( "@", false, path );
    }

    // Destroy the locks to our runtime management folders.
    if ( m_unpackRoot )
    {
        delete m_unpackRoot;

        m_unpackRoot = NULL;
    }
    
    if ( m_realtimeRoot )
    {
        delete m_realtimeRoot;

        m_realtimeRoot = NULL;
    }

    if ( m_fileRoot )
    {
        delete m_fileRoot;

        m_fileRoot = NULL;
    }

    if ( needDeletion )
    {
        // Delete all temporary files by deleting our entire folder structure.
        if ( CFileTranslator *sysTmpRoot = m_zipExtension.GetTempRoot() )
        {
            sysTmpRoot->Delete( path.c_str() );
        }
    }
}

CFileTranslator* CZIPArchiveTranslator::GetFileRoot( void )
{
    if ( !m_fileRoot )
    {
        // Attempt to get the handle to our temporary directory.
        CFileTranslator *sysTmpRoot = m_zipExtension.GetTempRoot();

        if ( sysTmpRoot )
        {
            // Create temporary storage
            filePath path;
            {
                std::stringstream number_stream;

                number_stream << sysTmpCnt++;

                std::string dirName( "/" );
                dirName += number_stream.str();
                dirName += "/";

                sysTmpRoot->CreateDir( dirName.c_str() );
                sysTmpRoot->GetFullPathFromRoot( dirName.c_str(), false, path );
            }

            m_fileRoot = fileSystem->CreateTranslator( path.c_str() );
        }
    }
    return m_fileRoot;
}

AINLINE CFileTranslator* AcquireDirectoryTranslator( CFileTranslator *fileRoot, const char *dirName )
{
    CFileTranslator *resultTranslator = NULL;

    bool canBeAcquired = true;

    if ( !fileRoot->Exists( dirName ) )
    {
        canBeAcquired = fileRoot->CreateDir( dirName );
    }

    if ( canBeAcquired )
    {
        filePath rootPath;

        bool gotRoot = fileRoot->GetFullPath( "", false, rootPath );

        if ( gotRoot )
        {
            resultTranslator = fileSystem->CreateTranslator( rootPath + dirName );
        }
    }

    return resultTranslator;
}

CFileTranslator* CZIPArchiveTranslator::GetUnpackRoot( void )
{
    if ( !m_unpackRoot )
    {
        CFileTranslator *fileRoot = GetFileRoot();

        if ( fileRoot )
        {
            m_unpackRoot = AcquireDirectoryTranslator( fileRoot, "unpack/" );
        }
    }
    return m_unpackRoot;
}

CFileTranslator* CZIPArchiveTranslator::GetRealtimeRoot( void )
{
    if ( !m_realtimeRoot )
    {
        CFileTranslator *fileRoot = GetFileRoot();

        if ( fileRoot )
        {
            m_realtimeRoot = AcquireDirectoryTranslator( fileRoot, "realtime/" );
        }
    }
    return m_realtimeRoot;
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

        {
            // Attempt to get a handle to the realtime root.
            CFileTranslator *realtimeRoot = GetRealtimeRoot();

            if ( !realtimeRoot )
                return NULL;

            if ( entry->archived && !entry->cached )
            {
                dstFile = realtimeRoot->Open( relPath.c_str(), "wb+" );

                Extract( *dstFile, *entry );
                entry->cached = true;
            }
            else
                dstFile = realtimeRoot->Open( relPath.c_str(), "rb+" );
        }

        break;
    case FILE_MODE_CREATE:
        {
            // Attempt to get a handle to the realtime root.
            CFileTranslator *realtimeRoot = GetRealtimeRoot();

            if ( !realtimeRoot )
                return NULL;

            entry = MakeDeriviateDir( *m_curDirEntry, tree ).MakeFile( name );

            if ( !entry )
                return NULL;

		    // Files start off clean
		    entry->Reset();

            dstFile = realtimeRoot->Open( relPath.c_str(), "wb+" );

            entry->cached = true;
        }
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

    union
    {
        CFileTranslator *unpackRoot;
        CFileTranslator *realtimeRoot;
    };

    if ( !srcEntry->cached )
    {
        unpackRoot = GetUnpackRoot();

        if ( !unpackRoot )
            return false;
    }
    else
    {
        realtimeRoot = GetRealtimeRoot();

        if ( !realtimeRoot )
            return false;
    }

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

            CFile *dstFile = unpackRoot->Open( dstEntry->relPath.c_str(), "wb" );

            FileSystem::StreamCopyCount( m_file, *dstFile, header.sizeCompressed );

            delete dstFile;
        }
        else
            unpackRoot->Copy( srcEntry->relPath.c_str(), dstEntry->relPath.c_str() );
    }
    else
    {
        dstEntry->cached = true;

        realtimeRoot->Copy( srcEntry->relPath.c_str(), dstEntry->relPath.c_str() );
    }

    return true;
}

bool CZIPArchiveTranslator::Rename( const char *src, const char *dst )
{
    // TODO: add directory support.
    file *entry = (file*)GetFileEntry( src );

    if ( !entry )
        return false;

    union
    {
        CFileTranslator *unpackRoot;
        CFileTranslator *realtimeRoot;
    };

    if ( !entry->cached )
    {
        unpackRoot = GetUnpackRoot();

        if ( !unpackRoot )
            return false;
    }
    else
    {
        realtimeRoot = GetRealtimeRoot();

        if ( !realtimeRoot )
            return false;
    }

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

            CFile *dstFile = unpackRoot->Open( dst, "wb" );

            FileSystem::StreamCopyCount( m_file, *dstFile, header.sizeCompressed );

            delete dstFile;
        }
        else
            unpackRoot->Rename( entry->relPath.c_str(), dst );
    }
    else
    {
        if ( !entry->locks.empty() )
            return false;

        realtimeRoot->Rename( entry->relPath.c_str(), dst );
    }

    // Give it a new name
    entry->name = fileName;

    _CreateDirTree( *m_curDirEntry, tree ).MoveTo( *entry );
    return true;
}

size_t CZIPArchiveTranslator::Size( const char *path ) const
{
    const file *entry = GetFileEntry( path );

    size_t preferableSize = 0;

    if ( entry )
    {
        if ( !entry->cached )
        {
            preferableSize = entry->sizeReal;
        }
        else
        {
            CFileTranslator *realtimeRoot = const_cast <CZIPArchiveTranslator*> ( this )->GetRealtimeRoot();

            if ( realtimeRoot )
            {
                preferableSize = realtimeRoot->Size( path );
            }
        }
    }
    return preferableSize;
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
    
    size_t realSize = 0;

    if ( !entry->cached )
    {
        realSize = entry->sizeReal;
    }
    else
    {
        CFileTranslator *realtimeRoot = const_cast <CZIPArchiveTranslator*> ( this )->GetRealtimeRoot();

        if ( realtimeRoot )
        {
            realSize = realtimeRoot->Size( entry->relPath.c_str() );
        }
    }
    stats->st_size = realSize;

    return true;
}

bool CZIPArchiveTranslator::ReadToBuffer( const char *path, std::vector <char>& output ) const
{
    // TODO.
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
    CFile *from = NULL;
    size_t comprSize = info.sizeCompressed;

    bool fromNeedClosing = false;

    if ( info.subParsed )
    {
        CFileTranslator *unpackRoot = GetUnpackRoot();

        if ( unpackRoot )
        {
            from = unpackRoot->Open( info.relPath.c_str(), "rb" );

            if ( from )
            {
                fromNeedClosing = true;
            }
        }
    }
    
    if ( !from )
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

    if ( fromNeedClosing )
    {
        delete from;
    }
}

void CZIPArchiveTranslator::CacheDirectory( const directory& dir )
{
    fileList::const_iterator fileIter = dir.files.begin();

    for ( ; fileIter != dir.files.end(); ++fileIter )
    {
        file *fileEntry = *fileIter;

        if ( fileEntry->cached || fileEntry->subParsed )
            continue;

        // Dump the compressed content
        CFileTranslator *unpackRoot = GetUnpackRoot();

        if ( unpackRoot )
        {
            _localHeader header;
            seekFile( *fileEntry, header );

            CFile *dst = unpackRoot->Open( fileEntry->relPath.c_str(), "wb" );

            FileSystem::StreamCopyCount( m_file, *dst, fileEntry->sizeCompressed );

            delete dst;

            fileEntry->subParsed = true;
        }
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

        bool canProcess = true;

        union
        {
            CFileTranslator *unpackRoot;
            CFileTranslator *realtimeRoot;
        };

        if ( !info.cached )
        {
            unpackRoot = GetUnpackRoot();

            if ( !unpackRoot )
            {
                canProcess = false;
            }
        }
        else
        {
            realtimeRoot = GetRealtimeRoot();

            if ( !realtimeRoot )
            {
                canProcess = false;
            }
        }

        if ( !canProcess )
            throw;

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

            CFile *src = unpackRoot->Open( info.relPath.c_str(), "rb" );

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

            CFile *src = dynamic_cast <CSystemFileTranslator*> ( realtimeRoot )->OpenEx( info.relPath.c_str(), "rb", FILE_FLAG_WRITESHARE );

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