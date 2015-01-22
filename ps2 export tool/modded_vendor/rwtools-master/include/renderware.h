#ifndef _RENDERWARE_H_
#define _RENDERWARE_H_
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <assert.h>

#ifdef DEBUG
	#define READ_HEADER(x)\
	header.read(rw);\
	if (header.type != (x)) {\
		cerr << filename << " ";\
		ChunkNotFound((x), rw.tellg());\
	}
#else
	#define READ_HEADER(x)\
	header.read(rw);
#endif

namespace rw
{

// Forward declaration.
struct Interface;

//typedef unsigned int uint;
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef float float32;

enum PLATFORM_ID
{
	PLATFORM_OGL = 2,
	PLATFORM_PS2    = 4,
	PLATFORM_XBOX   = 5,
	PLATFORM_D3D8   = 8,
	PLATFORM_D3D9   = 9,
	PLATFORM_PS2FOURCC = 0x00325350 /* "PS2\0" */
};
typedef enum PLATFORM_ID PLATFORM_ID;

enum CHUNK_TYPE
{
    CHUNK_NAOBJECT        = 0x0,
    CHUNK_STRUCT          = 0x1,
    CHUNK_STRING          = 0x2,
    CHUNK_EXTENSION       = 0x3,
    CHUNK_CAMERA          = 0x5,
    CHUNK_TEXTURE         = 0x6,
    CHUNK_MATERIAL        = 0x7,
    CHUNK_MATLIST         = 0x8,
    CHUNK_ATOMICSECT      = 0x9,
    CHUNK_PLANESECT       = 0xA,
    CHUNK_WORLD           = 0xB,
    CHUNK_SPLINE          = 0xC,
    CHUNK_MATRIX          = 0xD,
    CHUNK_FRAMELIST       = 0xE,
    CHUNK_GEOMETRY        = 0xF,
    CHUNK_CLUMP           = 0x10,
    CHUNK_LIGHT           = 0x12,
    CHUNK_UNICODESTRING   = 0x13,
    CHUNK_ATOMIC          = 0x14,
    CHUNK_TEXTURENATIVE   = 0x15,
    CHUNK_TEXDICTIONARY   = 0x16,
    CHUNK_ANIMDATABASE    = 0x17,
    CHUNK_IMAGE           = 0x18,
    CHUNK_SKINANIMATION   = 0x19,
    CHUNK_GEOMETRYLIST    = 0x1A,
    CHUNK_ANIMANIMATION   = 0x1B,
    CHUNK_HANIMANIMATION  = 0x1B,
    CHUNK_TEAM            = 0x1C,
    CHUNK_CROWD           = 0x1D,
    CHUNK_RIGHTTORENDER   = 0x1F,
    CHUNK_MTEFFECTNATIVE  = 0x20,
    CHUNK_MTEFFECTDICT    = 0x21,
    CHUNK_TEAMDICTIONARY  = 0x22,
    CHUNK_PITEXDICTIONARY = 0x23,
    CHUNK_TOC             = 0x24,
    CHUNK_PRTSTDGLOBALDATA = 0x25,
    CHUNK_ALTPIPE         = 0x26,
    CHUNK_PIPEDS          = 0x27,
    CHUNK_PATCHMESH       = 0x28,
    CHUNK_CHUNKGROUPSTART = 0x29,
    CHUNK_CHUNKGROUPEND   = 0x2A,
    CHUNK_UVANIMDICT      = 0x2B,
    CHUNK_COLLTREE        = 0x2C,
    CHUNK_ENVIRONMENT     = 0x2D,
    CHUNK_COREPLUGINIDMAX = 0x2E,

    CHUNK_MORPH           = 0x105,
    CHUNK_SKYMIPMAP       = 0x110,
    CHUNK_SKIN            = 0x116,
    CHUNK_PARTICLES       = 0x118,
    CHUNK_HANIM           = 0x11E,
    CHUNK_MATERIALEFFECTS = 0x120,
    CHUNK_ADCPLG          = 0x134,
    CHUNK_BINMESH         = 0x50E,
    CHUNK_NATIVEDATA      = 0x510,
    CHUNK_VERTEXFORMAT    = 0x510,

    CHUNK_PIPELINESET      = 0x253F2F3,
    CHUNK_SPECULARMAT      = 0x253F2F6,
    CHUNK_2DFX             = 0x253F2F8,
    CHUNK_NIGHTVERTEXCOLOR = 0x253F2F9,
    CHUNK_COLLISIONMODEL   = 0x253F2FA,
    CHUNK_REFLECTIONMAT    = 0x253F2FC,
    CHUNK_MESHEXTENSION    = 0x253F2FD,
    CHUNK_FRAME            = 0x253F2FE
};
typedef enum CHUNK_TYPE CHUNK_TYPE;

enum
{
	RASTER_AUTOMIPMAP = 0x1000,
	RASTER_PAL8 = 0x2000,
	RASTER_PAL4 = 0x4000,
	RASTER_MIPMAP = 0x8000
};

#pragma warning(push)
#pragma warning(disable: 4996)

struct LibraryVersion
{
    uint16 buildNumber;
    
    union
    {
        uint32 version;
        struct
        {
            uint8 rwRevMinor;
            uint8 rwRevMajor;
            uint8 rwLibMinor;
            uint8 rwLibMajor;
        };
    };

    inline bool operator ==( const LibraryVersion& right ) const
    {
        return
            this->buildNumber == right.buildNumber &&
            this->version == right.version;
    }

    std::string toString(void) const
    {
        std::string returnVer;

        char numBuf[ 10 ];

        // Zero it.
        numBuf[ sizeof( numBuf ) - 1 ] = '\0';

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwLibMajor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwLibMinor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwRevMajor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwRevMinor );

        returnVer += numBuf;

        // If we have a valid build number, append it.
        if ( this->buildNumber != 0xFFFF )
        {
            _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->buildNumber );

            returnVer += " (build: ";
            returnVer += numBuf;
            returnVer += ")";
        }

        return returnVer;
    }
};

#pragma warning(pop)

struct HeaderInfo
{
private:
	uint32 type;
	uint32 length;
	
    // Packed library version.
    struct PackedLibraryVersion
    {
        uint16 buildNumber;

        uint16 packedMinor : 8;
        uint16 pad : 2;
        uint16 packedMajor : 6;
    };

    PackedLibraryVersion packedVersion;

public:
	void read(std::istream &rw);
	uint32 write(std::ostream &rw);

    void setVersion(const LibraryVersion& version);
    LibraryVersion getVersion(void) const;

    void setType(uint32 type)               { this->type = type; }
    uint32 getType(void) const              { return this->type; }

    void setLength(uint32 length)           { this->length = length; }
    uint32 getLength(void) const            { return this->length; }
};

void ChunkNotFound(CHUNK_TYPE chunk, uint32 address);
uint32 writeInt8(int8 tmp, std::ostream &rw);
uint32 writeUInt8(uint8 tmp, std::ostream &rw);
uint32 writeInt16(int16 tmp, std::ostream &rw);
uint32 writeUInt16(uint16 tmp, std::ostream &rw);
uint32 writeInt32(int32 tmp, std::ostream &rw);
uint32 writeUInt32(uint32 tmp, std::ostream &rw);
uint32 writeFloat32(float32 tmp, std::ostream &rw);
int8 readInt8(std::istream &rw);
uint8 readUInt8(std::istream &rw);
int16 readInt16(std::istream &rw);
uint16 readUInt16(std::istream &rw);
int32 readInt32(std::istream &rw);
uint32 readUInt32(std::istream &rw);
float32 readFloat32(std::istream &rw);

std::string getChunkName(uint32 i);
/*
 * DFFs
 */

//#define NORMALSCALE (1.0/128.0)
//#define	VERTSCALE1 (1.0/128.0)	/* normally used */
//#define	VERTSCALE2 (1.0/1024.0)	/* used by objects with normals */
//#define	UVSCALE (1.0/4096.0)

enum
{ 
    FLAGS_TRISTRIP   = 0x01, 
    FLAGS_POSITIONS  = 0x02, 
    FLAGS_TEXTURED   = 0x04, 
    FLAGS_PRELIT     = 0x08, 
    FLAGS_NORMALS    = 0x10, 
    FLAGS_LIGHT      = 0x20, 
    FLAGS_MODULATEMATERIALCOLOR  = 0x40, 
    FLAGS_TEXTURED2  = 0x80
};

enum
{
	MATFX_BUMPMAP = 0x1,
	MATFX_ENVMAP = 0x2,
	MATFX_BUMPENVMAP = 0x3,
	MATFX_DUAL = 0x4,
	MATFX_UVTRANSFORM = 0x5,
	MATFX_DUALUVTRANSFORM = 0x6,
};

enum {
	FACETYPE_STRIP = 0x1,
	FACETYPE_LIST = 0x0
};

namespace KnownVersions
{
    enum eGameVersion
    {
        GTA3,
        VC_PS2,
        VC_PC,
        SA
    };

    LibraryVersion  getGameVersion( eGameVersion gameVer );
};

struct Frame
{
	float32 rotationMatrix[9];
	float32 position[3];
	int32 parent;

	/* Extensions */

	/* node name */
	std::string name;

	/* hanim */
	bool hasHAnim;
	uint32 hAnimUnknown1;
	int32 hAnimBoneId;
	uint32 hAnimBoneCount;
	uint32 hAnimUnknown2;
	uint32 hAnimUnknown3;
	std::vector<int32> hAnimBoneIds;
	std::vector<uint32> hAnimBoneNumbers;
	std::vector<uint32> hAnimBoneTypes;

	/* functions */
	void readStruct(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 writeStruct(std::ostream &dff);
	uint32 writeExtension(std::ostream &dff);

	void dump(uint32 index, std::string ind = "");

	Frame(void);
};

#include "renderware.txd.h"
#include "renderware.material.h"
#include "renderware.dff.h"
#include "renderware.file.h"

// Warning manager interface.
struct WarningManagerInterface abstract
{
    virtual void OnWarning( const std::string& message ) = 0;
};

struct RwException
{
    inline RwException( const std::string& msg )
    {
        this->message = msg;
    }

    std::string message;
};

// Palettization configuration.
enum ePaletteRuntimeType
{
    PALRUNTIME_NATIVE,      // use the palettizer that is embedded into rwtools
    PALRUNTIME_PNGQUANT     // use the libimagequant vendor
};

// DXT compression configuration.
enum eDXTCompressionMethod
{
    DXTRUNTIME_NATIVE,      // prefer our own logic
    DXTRUNTIME_SQUISH       // prefer squish
};

struct Interface
{
    Interface( void );
    ~Interface( void );

    void                SetVersion              ( LibraryVersion version );
    LibraryVersion      GetVersion              ( void ) const     { return this->version; }

    void                BeginDebug              ( void );
    void                ProcessDebug            ( void );

    void                SetFileInterface        ( FileInterface *fileIntf );
    FileInterface*      GetFileInterface        ( void );

    void                SetWarningManager       ( WarningManagerInterface *warningMan );
    void                SetWarningLevel         ( int level );
    int                 GetWarningLevel         ( void ) const;

    void                SetIgnoreSecureWarnings ( bool doIgnore );
    bool                GetIgnoreSecureWarnings ( void ) const;

    void                PushWarning             ( const std::string& message );

    void                SetPaletteRuntime       ( ePaletteRuntimeType palRunType );
    ePaletteRuntimeType GetPaletteRuntime       ( void ) const;

    void                    SetDXTRuntime       ( eDXTCompressionMethod dxtRunType );
    eDXTCompressionMethod   GetDXTRuntime       ( void ) const;

    void                SetFixIncompatibleRasters   ( bool doFix );
    bool                GetFixIncompatibleRasters   ( void ) const;

    void                SetDXTPackedDecompression   ( bool packedDecompress );
    bool                GetDXTPackedDecompression   ( void ) const;

private:
    LibraryVersion version;     // version of the output files (III, VC, SA)

    FileInterface *customFileInterface;

    WarningManagerInterface *warningManager;

    ePaletteRuntimeType palRuntimeType;
    eDXTCompressionMethod dxtRuntimeType;
    
    int warningLevel;
    bool ignoreSecureWarnings;

    bool fixIncompatibleRasters;
    bool dxtPackedDecompression;
};

// Global RenderWare interface.
// There is only one per library.
extern Interface rwInterface;

}

#endif
