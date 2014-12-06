#include <StdInc.h>

#include <algorithm>

namespace rw
{

#if 0
struct texInfoReg
{
    ps2GSRegisters::TEX0_REG tex0;
    ps2GSRegisters::TEX1_REG tex1;

    ps2GSRegisters::MIPTBP1_REG miptbp1;
    ps2GSRegisters::MIPTBP2_REG miptbp2;

    inline bool operator ==( const texInfoReg& right ) const
    {
        return
            this->tex0 == right.tex0 &&
            this->tex1 == right.tex1 &&
            this->miptbp1 == right.miptbp1 &&
            this->miptbp2 == right.miptbp2;
    }
};

struct sortToken_t
{
    uint32 width, height;
    uint32 depth;
    uint32 rasterFormat;

    inline bool operator ==( const sortToken_t& right ) const
    {
        return
            this->width == right.width &&
            this->height == right.height &&
            this->depth == right.depth &&
            this->rasterFormat == right.rasterFormat;
    }
};

typedef std::vector <texInfoReg> texregdebug_t;

typedef uniqueMap_t <sortToken_t, texregdebug_t> debuglist_t;

static debuglist_t _depthtexdebug;

static FileInterface::filePtr_t _debugEntriesFile = NULL;

static bool _dumpTextureFormats = true;
#endif

void NativeTexture::StartDebug( void )
{
#if 0
    // Generate a texture format database name.
    const char *fmtFileName = "txdfmt.dat";
    {
        uint32 engineVersion = rw::rwInterface.GetVersion();

        if ( engineVersion == rw::SA )
        {
            fmtFileName = "txdfmt_sa.dat";
        }
        else if ( engineVersion == rw::VCPS2 )
        {
            fmtFileName = "txdfmt_vc.dat";
        }
        else if ( engineVersion == rw::GTA3_1 ||
                  engineVersion == rw::GTA3_2 ||
                  engineVersion == rw::GTA3_3 ||
                  engineVersion == rw::GTA3_4 )
        {
            fmtFileName = "txdfmt_lc.dat";
        }
    }

    FileInterface *fileIntf = rw::rwInterface.GetFileInterface();

    if ( _dumpTextureFormats )
    {
        _debugEntriesFile = fileIntf->OpenStream( fmtFileName, "wb" );
    }
    else
    {
        // Process the known texture formats and generate data based on them.
        // Assert that our routines work correctly.
        FileInterface::filePtr_t entriesFile = fileIntf->OpenStream( fmtFileName, "rb" );

        if ( entriesFile )
        {
            while ( fileIntf->IsEOFStream( entriesFile ) == false )
            {
                _knownGPUTextureFormat knownFmt;

                size_t readCount = fileIntf->ReadStream( entriesFile, &knownFmt, sizeof( knownFmt ) );

                if ( readCount != sizeof( knownFmt ) )
                {
                    // Broken format file.
                    break;
                }

                // Verify this format.
                ps2GSRegisters calcUnk;

                ps2texnative::generatePS2GPUData(knownFmt.convBasics, calcUnk);

                // Create debug listings.
                ps2texnative::PerformDebugChecks(knownFmt.convBasics, knownFmt.gpuOut);
            }

            // Process debug listings.
            NativeTexture::DebugParameters();

            fileIntf->CloseStream( entriesFile );

            __asm nop
        }
    }
#endif
}

void NativeTexture::DebugParameters( void )
{
#if 0
    typedef std::vector <sortToken_t> sortList_t;

    typedef uniqueMap_t <texInfoReg, sortList_t> reverseLU_t;

    // Calculate the reverse lookup of parameters.
    reverseLU_t lookup;

    for ( debuglist_t::list_t::const_iterator iter = _depthtexdebug.pairs.begin(); iter != _depthtexdebug.pairs.end(); iter++ )
    {
        if ( iter->value.size() != 1 )
        {
            __asm nop
        }

        sortList_t& list = lookup[ iter->value.at( 0 ) ];

        list.push_back( iter->key );
    }

    __asm nop

    std::vector <dimData_t> avail_dims;

    // Calculate which dimensions never existed.
    for ( uint32 width = 2; width <= 1024; width *= 2 )
    {
        for ( uint32 height = 2; height <= 1024; height *= 2 )
        {
            bool hasDimension = false;

            for ( debuglist_t::list_t::const_iterator iter = _depthtexdebug.pairs.begin(); iter != _depthtexdebug.pairs.end(); iter++ )
            {
                const sortToken_t& theToken = iter->key;

                if ( theToken.width == width && theToken.height == height )
                {
                    hasDimension = true;
                    break;
                }
            }

            if ( !hasDimension )
            {
                dimData_t theDim;
                theDim.width = width;
                theDim.height = height;

                avail_dims.push_back( theDim );
            }
        }
    }

    __asm nop

#if 0
    // Calculate bitfield combinations that dimensions share.
    sharingFlags_t _flagshare_combine;

    for ( sharingFlags_t::list_t::const_iterator outer_iter = _flagshare.pairs.begin(); outer_iter != _flagshare.pairs.end(); outer_iter++ )
    {
        {
            uint32 shared_flag = outer_iter->key;

            sharingFlags_t::list_t::const_iterator sec_iter = outer_iter + 1;

            while ( sec_iter != _flagshare.pairs.end() )
            {
                shared_flag |= sec_iter->key;

                sortList_t& flagtab = _flagshare_combine[ shared_flag ];

                // Check which items hare this exact flag.
                for ( reverseLU_t::list_t::iterator iter = lookup.pairs.begin(); iter != lookup.pairs.end(); iter++ )
                {
                    if ( (iter->key.unk1 & shared_flag) == shared_flag )
                    {
                        for ( sortList_t::const_iterator inner_iter = iter->value.begin(); inner_iter != iter->value.end(); inner_iter++ )
                        {
                            flagtab.push_back( *inner_iter );
                        }
                    }
                }

                sec_iter++;
            }
        }

        {
            uint32 shared_flag = outer_iter->key;

            sharingFlags_t::list_t::const_iterator sec_iter = _flagshare.pairs.end();

            while ( sec_iter != _flagshare.pairs.begin() - 1 )
            {
                shared_flag |= sec_iter->key;

                sortList_t& flagtab = _flagshare_combine[ shared_flag ];

                // Check which items hare this exact flag.
                for ( reverseLU_t::list_t::iterator iter = lookup.pairs.begin(); iter != lookup.pairs.end(); iter++ )
                {
                    if ( (iter->key.unk1 & shared_flag) == shared_flag )
                    {
                        for ( sortList_t::const_iterator inner_iter = iter->value.begin(); inner_iter != iter->value.end(); inner_iter++ )
                        {
                            flagtab.push_back( *inner_iter );
                        }
                    }
                }

                sec_iter--;
            }
        }
    }
#endif

    __asm nop

    // Close down the debug entries file.
    if ( _debugEntriesFile )
    {
        rw::rwInterface.GetFileInterface()->CloseStream( _debugEntriesFile );

        _debugEntriesFile = NULL;
    }

    __asm nop
#endif
}

#if 0
void ps2texnative::PerformDebugChecks( const ps2texnative::gpuTextureFormat& texFmt, const ps2GSRegisters& textureMeta )
{
    // PARAMETER DEBUG.
    sortToken_t stoken;
    stoken.width = texFmt.width;
    stoken.height = texFmt.height;
    stoken.depth = texFmt.depth;
    stoken.rasterFormat = (texFmt.rasterFormat & ~(RASTER_PALMASK | 0x20000 | 0x10000 | 0x8000));

    texInfoReg info;
    info.tex0 = textureMeta.tex0;
    info.tex1 = textureMeta.tex1;
    info.miptbp1 = textureMeta.miptbp1;
    info.miptbp2 = textureMeta.miptbp2;

    texregdebug_t& _texregdebug = _depthtexdebug[ stoken ];

    if ( std::find( _texregdebug.begin(), _texregdebug.end(), info ) == _texregdebug.end() )
    {
        if ( _texregdebug.size() != 0 )
        {
            __asm nop
        }

        _texregdebug.push_back( info );

        __asm nop

        // Write the format into the debug container.
        if ( _debugEntriesFile )
        {
            _knownGPUTextureFormat fileEntry;
            fileEntry.convBasics = texFmt;

            fileEntry.gpuOut.tex0 = textureMeta.tex0;
            fileEntry.gpuOut.tex1 = textureMeta.tex1;

            fileEntry.gpuOut.miptbp1 = textureMeta.miptbp1;
            fileEntry.gpuOut.miptbp2 = textureMeta.miptbp2;

            rw::rwInterface.GetFileInterface()->WriteStream( _debugEntriesFile, &fileEntry, sizeof(fileEntry) );
        }
    }

    //NativeTexture::DebugParameters();
}
#endif

};