/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.zip.h
*  PURPOSE:     ZIP archive filesystem
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_ZIP_
#define _FILESYSTEM_ZIP_

class CArchiveFileTranslator : public CSystemPathTranslator
{
    friend class CFileSystem;
    friend class CArchiveFile;
public:
                    CArchiveFileTranslator( CFile& file );
                    ~CArchiveFileTranslator();

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

private:
    void            ReadFiles( unsigned int count );

    struct file;

    class stream abstract : public CFile
    {
        friend class CArchiveFileTranslator;
    public:
        stream( CArchiveFileTranslator& zip, file& info, CFile& sysFile ) : m_info( info ), m_archive( zip ), m_sysFile( sysFile )
        { }

        ~stream()
        {
            delete &m_sysFile;
        }

        const filePath& GetPath() const
        {
            return m_path;
        }

    private:
        CFile&                      m_sysFile;
        CArchiveFileTranslator&     m_archive;
        file&                       m_info;
        filePath                    m_path;
    };

    struct file
    {
        filePath        name;
        unsigned short  version;
        unsigned short  reqVersion;
        unsigned short  flags;
        unsigned short  compression;
        unsigned short  modTime;
        unsigned short  modDate;

        unsigned int    crc32;
        size_t          sizeCompressed;
        size_t          sizeReal;

        unsigned short  diskID;
        unsigned short  internalAttr;
        unsigned int    externalAttr;
        size_t          localHeaderOffset;

        std::string     extra;
        std::string     comment;

        bool            archived;
        bool            cached;
        class stream*   locker;

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
    };

    // We need to cache data on the disk
    void            Extract( CFile& dstFile, file& info );

    class fileDeflate : public stream
    {
        friend class CArchiveFileTranslator;
    public:
                        fileDeflate( CArchiveFileTranslator& zip, file& info, CFile& sysFile );
                        ~fileDeflate();

        size_t          Read( void *buffer, size_t sElement, unsigned long iNumElements );
        size_t          Write( const void *buffer, size_t sElement, unsigned long iNumElements );
        int             Seek( long iOffset, int iType );
        long            Tell() const;
        bool            IsEOF() const;
        bool            Stat( struct stat *stats ) const;
        void            PushStat( const struct stat *stats );
        size_t          GetSize() const;
        void            Flush();
        bool            IsReadable() const;
        bool            IsWriteable() const;

    private:
        inline void     Focus();

        bool            m_writeable;
        bool            m_readable;
    };

    typedef std::list <file*> fileList;

    struct directory
    {
        filePath name;

        typedef std::list <directory*> subDirs;

        fileList files;
        subDirs children;

        directory* parent;

        inline directory*  FindDirectory( const filePath& dirName ) const
        {
            subDirs::const_iterator iter;

            for ( iter = children.begin(); iter != children.end(); iter++ )
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

            dir = new directory;
            dir->name = dirName;
            dir->parent = this;

            children.push_back( dir );
            return *dir;
        }

        inline file&    AddFile( const filePath& fileName )
        {
            file& entry = *new file;
            entry.name = fileName;

            files.push_back( &entry );
            return entry;
        }

        inline file*    GetFile( const filePath& fileName )
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); iter++ )
            {
                if ( (*iter)->name == fileName )
                    return *iter;
            }

            return NULL;
        }

        inline void     RemoveFile( file& entry )
        {
            delete &entry;

            files.remove( &entry );
        }

        inline bool     RemoveFile( const filePath& fileName )
        {
            fileList::const_iterator iter = files.begin();

            for ( ; iter != files.end(); iter++ )
            {
                if ( (*iter)->name == fileName )
                {
                    delete *iter;

                    files.remove( *iter );
                    return true;
                }
            }
            return false;
        }
    };

    directory m_root;

    inline directory*   GetDirTree( directory& root, dirTree::const_iterator& iter, dirTree::const_iterator& end )
    {
        directory *curDir = &root;

        for ( ; iter != end; iter++ )
        {
            if ( !( curDir = root.FindDirectory( *iter ) ) )
                return NULL;
        }

        return curDir;
    }

    inline directory*   GetDirTree( const dirTree& tree )
    {
        return GetDirTree( m_root, tree.begin(), tree.end() );
    }

    inline directory*   GetDeriviateDir( directory& root, const dirTree& tree );
    inline directory&   MakeDeriviateDir( directory& root, const dirTree& tree );

    directory&      _CreateDirTree( directory& root, const dirTree& tree );

    struct dataDescriptor
    {
        unsigned int    crc32;
        size_t          sizeCompressed;
        size_t          sizeReal;
    };

    struct localFileHeader
    {
        unsigned int    signature;
        unsigned short  version;
        unsigned short  flags;
        unsigned short  compression;
        unsigned short  modTime;
        unsigned short  modDate;

        dataDescriptor  desc;
        
        //sizefname 16
        //sizeextra 16
        std::string     name;
        std::string     extra;
    };

    struct extraData
    {
        unsigned int    signature;
        size_t          size;

        //data
    };

    struct centralFileHeader
    {
        unsigned int    signature;
        unsigned short  version;
        unsigned short  reqVersion;
        unsigned short  flags;
        unsigned short  compression;
        unsigned short  modTime;
        unsigned short  modData;

        dataDescriptor  desc;

        //filename 16
        //extra 16
        //comment 16

        unsigned short  diskID;
        unsigned short  internalAttr;
        unsigned int    externalAttr;
        size_t          localHeaderOffset;

        std::string     name;
        std::string     extra;
        std::string     comment;
    };

    struct centralDirectoryEnd
    {
        unsigned int    signature;
        unsigned short  diskID;
        unsigned short  diskAlign;
        unsigned short  entries;
        unsigned short  totalEntries;
        size_t          centralDirectorySize;
        size_t          centralDirectoryOffset;

        //commentlen 16

        std::string     globalComment;
    };

    CFile&      m_file;
    directory*  m_curDirEntry;

    CFileTranslator*    m_fileRoot;
};

#endif //_FILESYSTEM_ZIP_