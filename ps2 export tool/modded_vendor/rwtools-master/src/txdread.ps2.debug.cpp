#include <StdInc.h>

#include "txdread.ps2.hxx"

#include <algorithm>
#include <map>

namespace rw
{

struct texInfoReg
{
    std::string texName;
    uint32 size;
};

typedef std::vector <texInfoReg> deblist_t;

static std::map <uint32, deblist_t> addrSort;


void NativeTexturePS2::PerformDebugChecks(const textureMetaDataHeader& textureMeta) const
{
    if ( this->requiresHeaders == false )
        return;

    // PARAMETER DEBUG.
    texInfoReg info;
    //info.texName = this->parent->name;
    info.size = textureMeta.combinedGPUDataSize / 64;

    deblist_t& list = addrSort[ textureMeta.tex0.textureBasePointer ];

    list.push_back( info );

    //NativeTexture::DebugParameters();
}

};