/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.zip.internal.h
*  PURPOSE:     Master header of ZIP archive filesystem private modules
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_ZIP_PRIVATE_MODULES_
#define _FILESYSTEM_ZIP_PRIVATE_MODULES_

// Checksums.
#define ZIP_SIGNATURE               0x06054B50
#define ZIP_FILE_SIGNATURE          0x02014B50
#define ZIP_LOCAL_FILE_SIGNATURE    0x04034B50

#include <time.h>

#pragma warning(push)
#pragma warning(disable:4250)

class CZIPArchiveTranslator : public CSystemPathTranslator, public CArchiveTranslator
{
    friend struct zipExtension;
    friend class CArchiveFile;
public:
                    CZIPArchiveTranslator( zipExtension& theExtension, CFile& file );
                    ~CZIPArchiveTranslator( void );

    bool            WriteData( const char *path, const char *buffer, size_t size );
    bool            CreateDir( const char *path );
    CFile*          Open( const char *path, const char *mode );
    bool            Exists( const char *path ) const;
    bool            Delete( const char *path );
    bool            Copy( const char *src, const char *dst );
    bool            Rename( const char *src, const char *dst );
    size_t          Size( const char *path ) const;
    bool            Stat( const char *path, struct stat *stats ) const;
    bool            ReadToBuffer( const char *path, std::vector <char>& output ) const;

    bool            ChangeDirectory( const char *path );

    void            ScanDirectory( const char *directory, const char *wildcard, bool recurse,
                        pathCallback_t dirCallback,
                        pathCallback_t fileCallback,
                        void *userdata ) const;

    void            GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const;
    void            GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const;

    void            Save();

    // Members.
    zipExtension&   m_zipExtension;
    CFile&          m_file;

#pragma pack(1)
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
#pragma pack()

private:
    void            ReadFiles( unsigned int count );

public:
    struct file;
    struct directory;

private:
    class stream abstract : public CFile
    {
        friend class CZIPArchiveTranslator;
    public:
        stream( CZIPArchiveTranslator& zip, file& info, CFile& sysFile ) : m_sysFile( sysFile ), m_archive( zip ), m_info( info )
        {
            info.locks.push_back( this );
        }

        ~stream( void )
        {
            m_info.locks.remove( this );

            delete &m_sysFile;
        }

        const filePath& GetPath( void ) const
        {
            return m_path;
        }

    private:
        CFile&                      m_sysFile;
        CZIPArchiveTranslator&      m_archive;
        file&                       m_info;
        filePath                    m_path;
    };

public:
    struct fsActiveEntry
    {
        fsActiveEntry( const filePath& name, const filePath& relPath ) : name( name ), relPath( relPath )
        {
            return;
        }

        filePath        name;
        filePath        relPath;

        unsigned short  version;
        unsigned short  reqVersion;
        unsigned short  flags;

        unsigned short  diskID;

        unsigned short  modTime;
        unsigned short  modDate;

        size_t          localHeaderOffset;

        std::string     extra;
        std::string     comment;

        inline _localHeader ConstructLocalHeader( void ) const
        {
            _localHeader header;
            header.signature = ZIP_LOCAL_FILE_SIGNATURE;
            header.modTime = modTime;
            header.modDate = modDate;
            header.nameLen = relPath.size();
            header.commentLen = comment.size();
            return header;
        }

        inline _centralFile ConstructCentralHeader( void ) const
        {
            _centralFile header;
            header.signature = ZIP_FILE_SIGNATURE;
            header.version = version;
            header.reqVersion = reqVersion;
            header.flags = flags;
            //header.compression
            header.modTime = modTime;
            header.modDate = modDate;
            //header.crc32
            //header.sizeCompressed
            //header.sizeReal
            header.nameLen = relPath.size();
            header.extraLen = extra.size();
            header.commentLen = comment.size();
            header.diskID = diskID;
            //header.internalAttr
            //header.externalAttr
            header.localHeaderOffset = localHeaderOffset;
            return header;
        }

        inline void AllocateArchiveSpace( CZIPArchiveTranslator *archive, _localHeader& entryHeader, size_t& sizeCount )
        {
            // Update the offset.
            localHeaderOffset = archive->m_file.Tell() - archive->m_structOffset;

            sizeCount += sizeof( entryHeader ) + entryHeader.nameLen + entryHeader.commentLen;
        }

        inline void Reset( void )
        {
#ifdef _WIN32
            version = 10;
#else
            version = 0x03; // Unix
#endif //_WIN32
            reqVersion = 0x14;
            flags = 0;

            UpdateTime();

            diskID = 0;

            extra.clear();
            comment.clear();
        }

        inline void SetModTime( const tm& date )
        {
            unsigned short year = date.tm_year;

            if ( date.tm_year > 1980 )
                year -= 1980;
            else if ( date.tm_year > 80 )
                year -= 80;

            modDate = date.tm_mday | ((date.tm_mon + 1) << 5) | (year << 9);
            modTime = date.tm_sec >> 1 | (date.tm_min << 5) | (date.tm_hour << 11);
        }

        inline void GetModTime( tm& date ) const
        {
            date.tm_mday = modDate & 0x1F;
            date.tm_mon = ((modDate & 0x1E0) >> 5) - 1;
            date.tm_year = ((modDate & 0x0FE00) >> 9) + 1980;

            date.tm_hour = (modTime & 0xF800) >> 11;
            date.tm_min = (modTime & 0x7E0) >> 5;
            date.tm_sec = (modTime & 0x1F) << 1;

            date.tm_wday = 0;
            date.tm_yday = 0;
        }

        virtual void UpdateTime( void )
        {
            time_t curTime = time( NULL );
            SetModTime( *gmtime( &curTime ) );
        }
    };

    struct file : public fsActiveEntry
    {
        file( const filePath& name ) : fsActiveEntry( name, filePath() )
        {
            return;
        }

        unsigned short  compression;

        unsigned int    crc32;
        size_t          sizeCompressed;
        size_t          sizeReal;

        unsigned short  internalAttr;
        unsigned int    externalAttr;

        bool            archived;
        bool            cached;
        bool            subParsed;

        typedef std::list <stream*> lockList;
        lockList        locks;
        directory*      dir;

        inline void Reset( void )
        {
            // Common reset.
            fsActiveEntry::Reset();

            // File-specific reset.
            compression = 8;

            crc32 = 0;
            sizeCompressed = 0;
            sizeReal = 0;
            internalAttr = 0;
            externalAttr = 0;

            archived = false;
            cached = false;
            subParsed = false;
        }

        inline bool IsNative( void ) const
        {
#ifdef _WIN32
            return version == 10;
#else
            return version == 0x03; // Unix
#endif //_WIN32
        }

        inline void UpdateTime( void )
        {
            fsActiveEntry::UpdateTime();

            // Update the time of its parent directories.
            dir->UpdateTime();
        }
    };

private:
    inline void seekFile( const file& info, _localHeader& header );

    // We need to cache data on the disk
    void            Extract( CFile& dstFile, file& info );

    inline const file*  GetFileEntry( const char *path ) const
    {
        dirTree tree;
        bool isFile;

        if ( !GetRelativePathTree( path, tree, isFile ) || !isFile )
            return NULL;

        filePath fileName = tree[ tree.size() - 1 ];
        tree.pop_back();

        const directory *dir = GetDeriviateDir( *m_curDirEntry, tree );

        if ( !dir )
            return NULL;

        return dir->GetFile( fileName );
    }

    class fileDeflate : public stream
    {
        friend class CZIPArchiveTranslator;
    public:
                        fileDeflate( CZIPArchiveTranslator& zip, file& info, CFile& sysFile );
                        ~fileDeflate( void );

        size_t          Read( void *buffer, size_t sElement, unsigned long iNumElements );
        size_t          Write( const void *buffer, size_t sElement, unsigned long iNumElements );
        int             Seek( long iOffset, int iType );
        long            Tell( void ) const;
        bool            IsEOF( void ) const;
        bool            Stat( struct stat *stats ) const;
        void            PushStat( const struct stat *stats );
        void            SetSeekEnd( void );
        size_t          GetSize( void ) const;
        void            Flush( void );
        bool            IsReadable( void ) const;
        bool            IsWriteable( void ) const;

    private:
        inline void     Focus( void );

        bool            m_writeable;
        bool            m_readable;
    };

public:
    typedef std::list <file*> fileList;

    struct directory : public fsActiveEntry
    {
        directory( filePath fileName, filePath path ) : fsActiveEntry( fileName, path )
        {
            return;
        }

        ~directory( void )
        {
            subDirs::iterator iter = children.begin();

            for ( ; iter != children.end(); ++iter )
                delete *iter;

            fileList::iterator fileIter = files.begin();

            for ( ; fileIter != files.end(); ++fileIter )
                delete *fileIter;
        }

        typedef std::list <directory*> subDirs;

        fileList files;
        subDirs children;

        directory* parent;

        inline directory*  FindDirectory( const filePath& dirName ) const
        {
            subDirs::const_iterator iter;

            for ( iter = children.begin(); iter != children.end(); ++iter )
            {
                if ( (*iter)->name == dirName )
                    return *iter;
            }

            return NULL;
        }

        inline directory&  GetDirectory( const filePath& dirName )
        {
            directory *dir = FindDirectory( dirName );

            if ( dir )
                return *dir;

            dir = new directory( dirName, relPath + dirName + "/" );
            dir->name = dirName;
            dir->parent = this;
            dir->Reset();

            children.push_back( dir );
            return *dir;
        }

        inline void     PositionFile( file& entry )
        {
            entry.relPath = relPath;
            entry.relPath += entry.name;
        }

        inline file&    AddFile( const filePath& fileName )
        {
            file& entry = *new file( fileName );
			entry.flags = 0;

            PositionFile( entry );

            entry.cached = false;
            entry.subParsed = false;
            entry.archived = false;
            entry.dir = this;

            files.push_back( &entry );
            return entry;
        }

        inline void     UnlinkFile( file& entry )
        {
            files.remove( &entry );
        }

        inline void     MoveTo( file& entry )
        {
            entry.dir->UnlinkFile( entry );

            entry.dir = this;

            files.push_back( &entry );

            PositionFile( entry );
        }

        inline bool     IsLocked( void ) const
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); ++iter )
            {
                if ( !(*iter)->locks.empty() )
                    return true;
            }

            return false;
        }

        inline file*    GetFile( const filePath& fileName )
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); ++iter )
            {
                if ( (*iter)->name == fileName )
                    return *iter;
            }

            return NULL;
        }

        inline file*    MakeFile( const filePath& fileName )
        {
            file *entry = GetFile( fileName );

            if ( entry )
            {
                if ( !entry->locks.empty() )
                    return NULL;

                entry->name = fileName;
                entry->Reset();
                return entry;
            }

            return &AddFile( fileName );
        }

        inline const file*  GetFile( const filePath& fileName ) const
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); ++iter )
            {
                if ( (*iter)->name == fileName )
                    return *iter;
            }

            return NULL;
        }

        inline bool     RemoveFile( file& entry )
        {
            if ( !entry.locks.empty() )
                return false;

            delete &entry;

            UnlinkFile( entry );
            return true;
        }

        inline bool     RemoveFile( const filePath& fileName )
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); ++iter )
            {
                if ( (*iter)->name == fileName )
                    return RemoveFile( **iter );
            }
            return false;
        }

        inline bool     IsEmpty( void ) const
        {
            return ( files.empty() && children.empty() );
        }

        inline bool     NeedsWriting( void ) const
        {
            return ( IsEmpty() || comment.size() != 0 || extra.size() != 0 );
        }

        inline void     UpdateTime( void )
        {
            fsActiveEntry::UpdateTime();

            // Update the time of its parent directories.
            if ( parent )
            {
                parent->UpdateTime();
            }
        }
    };

private:
    directory m_rootDir;

    void            CacheDirectory( const directory& dir );
    void            SaveDirectory( directory& dir, size_t& size );
    unsigned int    BuildCentralFileHeaders( const directory& dir, size_t& size );

    inline const directory* GetDirTree( const directory& root, dirTree::const_iterator iter, dirTree::const_iterator end ) const
    {
        const directory *curDir = &root;

        for ( ; iter != end; ++iter )
        {
            if ( !( curDir = root.FindDirectory( *iter ) ) )
                return NULL;
        }

        return curDir;
    }

    inline const directory* GetDirTree( const dirTree& tree ) const
    {
        return GetDirTree( this->m_rootDir, tree.begin(), tree.end() );
    }

    const directory*    GetDeriviateDir( const directory& root, const dirTree& tree ) const;
    directory&          MakeDeriviateDir( directory& root, const dirTree& tree );

    directory&          _CreateDirTree( directory& root, const dirTree& tree );

    struct extraData
    {
        unsigned int    signature;
        size_t          size;

        //data
    };

    directory*  m_curDirEntry;
    std::string m_comment;

    CFileTranslator*    GetFileRoot         ( void );
    CFileTranslator*    GetUnpackRoot       ( void );
    CFileTranslator*    GetRealtimeRoot     ( void );

    // Runtime management file directories.
    // They are acquired once they are required.
    CFileTranslator*    m_fileRoot;
    CFileTranslator*    m_unpackRoot;
    CFileTranslator*    m_realtimeRoot;

    size_t      m_structOffset;
};

#pragma warning(pop)

#endif //_FILESYSTEM_ZIP_PRIVATE_MODULES_