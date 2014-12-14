#include <StdInc.h>

#include "txdread.ps2.hxx"

#include <algorithm>

namespace rw
{

struct texInfoReg
{
    uint16 offX, offY;

    inline bool operator ==( const texInfoReg& right ) const
    {
        return
            this->offX == right.offX &&
            this->offY == right.offY;
    }
};

struct sortToken_t
{
    uint32 width, height;
    uint32 depth;
    eRasterFormat rasterFormat;
    ePaletteType paletteType;

    inline bool operator ==( const sortToken_t& right ) const
    {
        return
            this->width == right.width &&
            this->height == right.height &&
            this->depth == right.depth &&
            this->rasterFormat == right.rasterFormat &&
            this->paletteType == right.paletteType;
    }
};

typedef std::vector <texInfoReg> texregdebug_t;

typedef uniqueMap_t <sortToken_t, texregdebug_t> debuglist_t;

static debuglist_t _depthtexdebug;

void NativeTexture::StartDebug( void )
{
    return;
}

void NativeTexture::DebugParameters( void )
{
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
}

void NativeTexturePS2::PerformDebugChecks( void ) const
{
    if ( this->paletteType != PALETTE_8BIT )
        return;

    if ( this->requiresHeaders == false )
        return;

    // PARAMETER DEBUG.
    sortToken_t stoken;
    stoken.width = this->swizzleWidth[ 0 ];
    stoken.height = this->swizzleHeight[ 0 ];
    stoken.depth = this->mipmapCount;
    stoken.rasterFormat = parent->rasterFormat;
    stoken.paletteType = this->paletteType;

    texInfoReg info;
    info.offX = this->palUnknowns.destX;
    info.offY = this->palUnknowns.destY;

    texregdebug_t& _texregdebug = _depthtexdebug[ stoken ];

    if ( std::find( _texregdebug.begin(), _texregdebug.end(), info ) == _texregdebug.end() )
    {
        if ( _texregdebug.size() != 0 )
        {
            __asm nop
        }

        _texregdebug.push_back( info );

        __asm nop
    }

    //NativeTexture::DebugParameters();
}

};