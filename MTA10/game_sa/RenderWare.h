/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare.h
*  PURPOSE:     RenderWare definitions
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is � Criterion Software
*
*****************************************************************************/

#ifndef __RENDERWARE_COMPAT
#define __RENDERWARE_COMPAT

#include <../sdk/game/RenderWare_shared.h>

/*****************************************************************************/
/** RenderWare rendering types                                              **/
/*****************************************************************************/

// RenderWare definitions
#define RW_STRUCT_ALIGN           ((int)((~((unsigned int)0))>>1))
#define RW_TEXTURE_NAME_LENGTH    32
#define RW_MAX_TEXTURE_COORDS     8

// RenderWare extensions
#define RW_EXTENSION_HANIM  0x253F2FB

// Yet to analyze
typedef void   RpWorld;

typedef RwCamera *(*RwCameraPreCallback) (RwCamera * camera);
typedef RwCamera *(*RwCameraPostCallback) (RwCamera * camera);
typedef RpAtomic *(*RpAtomicCallback) (RpAtomic * atomic);
typedef RpClump  *(*RpClumpCallback) (RpClump * clump, void *data);

// RenderWare enumerations
enum RwPrimitiveType
{
    PRIMITIVE_NULL = 0,
    PRIMITIVE_LINE_SEGMENT = 1,
    PRIMITIVE_LINE_SEGMENT_CONNECTED = 2,
    PRIMITIVE_TRIANGLE = 3,
    PRIMITIVE_TRIANGLE_STRIP = 4,
    PRIMITIVE_TRIANGLE_FAN = 5,
    PRIMITIVE_POINT = 6,
    PRIMITIVE_LAST = RW_STRUCT_ALIGN
};
enum RwCameraType
{
    RW_CAMERA_NULL = 0,
    RW_CAMERA_PERSPECTIVE = 1,
    RW_CAMERA_ORTHOGRAPHIC = 2,
    RW_CAMERA_LAST = RW_STRUCT_ALIGN
};
enum RpAtomicFlags
{
    ATOMIC_COLLISION = 1,
    ATOMIC_VISIBLE = 4,
    ATOMIC_LAST = RW_STRUCT_ALIGN
};
enum RwRasterLockFlags
{
    RASTER_LOCK_WRITE = 1,
    RASTER_LOCK_READ = 2,
    RASTER_LOCK_LAST = RW_STRUCT_ALIGN
};
enum RwTransformOrder
{
    TRANSFORM_INITIAL = 0,
    TRANSFORM_BEFORE = 1,
    TRANSFORM_AFTER = 2,
    TRANSFORM_LAST = RW_STRUCT_ALIGN
};
enum RpLightType
{
    LIGHT_TYPE_NULL = 0,
    
    LIGHT_TYPE_DIRECTIONAL = 1,
    LIGHT_TYPE_AMBIENT = 2,
    
    LIGHT_TYPE_POINT = 0x80,
    LIGHT_TYPE_SPOT_1 = 0x81,
    LIGHT_TYPE_SPOT_2 = 0x82,

    LIGHT_TYPE_LAST = RW_STRUCT_ALIGN
};
enum RpLightFlags
{
    LIGHT_ILLUMINATES_ATOMICS = 1,
    LIGHT_ILLUMINATES_GEOMETRY = 2,
    LIGHT_FLAGS_LAST = RW_STRUCT_ALIGN
};
enum eRwType
{
    RW_NULL,
    RW_ATOMIC,
    RW_CLUMP,
    RW_TXD = 6
};

// RenderWare/plugin base types
class RwObject
{
public:
    unsigned char   type;
    unsigned char   subtype;
    unsigned char   flags;
    unsigned char   privateFlags;
    class RwFrame*  m_parent;                // should be RwFrame with RpClump
};
struct RwVertex
{
    RwV3d        position;
    RwV3d        normal;
    unsigned int color;
    float        u,v;
};
template < class type >
struct RwListEntry
{
    type *next,*prev;
};
template < class type >
struct RwList
{
    RwListEntry <type> root;
};
class RwExtensionInterface
{
public:
    unsigned int            m_id;           
    unsigned int            m_unknown;          // 4
    unsigned int            m_structSize;       // 8
    void*                   m_apply;            // 12
    void*                   m_callback2;        // 16
    void*                   m_interpolate;      // 20
    void*                   m_callback4;        // 24
    void*                   m_callback5;        // 28
    void*                   m_callback6;        // 32
    void*                   m_callback7;        // 36
    void*                   m_callback8;        // 40
    size_t                  m_internalSize;     // 44
};
class RwExtension
{
public:
    RwExtensionInterface*   m_extension;    // 0
    unsigned int            m_count;        // 4
    size_t                  m_size;         // 8
    unsigned int            m_unknown;      // 12

    void*                   m_data;         // 16
    void*                   m_internal;     // 20
};
class RwRenderLink
{
public:
    unsigned char           m_flags;        
    BYTE                    m_pad[3];       // 1
    CVector                 m_position;     // 4
    void*                   m_context;      // 16
    int                     m_id;           // 20
};

#define BONE_ROOT           0x08

class RwStaticGeometry
{
public:
    RwStaticGeometry();

    unsigned int            m_unknown2;     // 0
    unsigned int            m_unknown;      // 4
    unsigned int            m_count;        // 8
    unsigned int            m_unknown3;     // 12
    RwRenderLink*           m_link;         // 16

    RwRenderLink*           AllocateLink( unsigned int count );
    void                    ForAllLinks( void (*callback)( RwRenderLink *link, void *data ), void *data );
};
class RwBoneInfo
{
public:
    unsigned int            m_index;
    BYTE                    m_pad[4];
    unsigned int            m_flags;
    DWORD                   m_pad2;
};
class RpSkeleton
{
public:
    unsigned int            m_boneCount;
    unsigned int            m_unknown;      // 4
    void*                   m_unknown2;     // 8
    RwMatrix*               m_boneMatrices; // 12
    unsigned int            m_unknown3;     // 16
    void*                   m_unknown5;     // 20
    RwV4d                   m_vertexInfo;   // 24
    unsigned int            m_unknown6;     // 28
    RpSkeletonEx*           m_data;         // 32
    BYTE                    m_pad[28];      // 36
    unsigned char*          m_boneParent2;  // 64
};
class RwAnimInfo    // dynamic
{
    void*                   m_unknown;      // 0
    CVector                 m_offset;       // 4
    BYTE                    m_pad[12];      // 16
};
class RpAnimation
{
public:
    RwExtension*            m_ext;
    BYTE                    m_pad[32];      // 4
    size_t                  m_infoSize;     // 36
    BYTE                    m_pad2[36];     // 40

    // Dynamically extended data
    RwAnimInfo              m_info[32];
};
class RpAnimHierarchy
{
public:
    unsigned int            m_flags;        // 0
    unsigned int            m_boneCount;    // 4
    BYTE                    m_pad[8];       // 8
    RpBoneInfo*             m_boneInfo;     // 16
    unsigned int            m_unknown4;     // 20
    RpAnimHierarchy*        m_this;         // 24
    unsigned char           m_unknown;      // 28
    unsigned char           m_unknown2;     // 29
    unsigned short          m_unknown3;     // 30
    RpAnimation*            m_anim;         // 32
};
class RwFrame : public RwObject
{
public:
    void*                   m_pad1;         // 8
    void*                   m_pad2;         // 12
    RwMatrix                m_modelling;    // 16
    RwMatrix                m_ltm;          // 80
    RwList <RwObject>       m_objects;      // 144
    RwFrame*                m_child;        // 152
    RwFrame*                m_next;         // 156
    RwFrame*                m_root;         // 160

    // Rockstar Frame extension (0x253F2FE) (24 bytes)
    RpAnimHierarchy*        m_anim;             // 164
    BYTE                    m_pluginData[4];    // 168
    char                    m_nodeName[16];     // 172

    BYTE                    m_pad3[8];      // 188
    unsigned int            m_hierarchyId;  // 196

    unsigned int            CountChildren();
    bool                    ForAllChildren( bool (*callback)( RwFrame *frame, void *data ), void *data );
    RwFrame*                FindFreeChildByName( const char *name );
    RwFrame*                FindChildByName( const char *name );

    RpAnimHierarchy*        GetAnimHierarchy();
};
class RwTexDictionary : public RwObject
{
public:
    RwList <RwTexture>              textures;
    RwListEntry <RwTexDictionary>   globalTXDs;
};
class RwTexture
{
    RwRaster*                   raster;
    RwTexDictionary*            txd;
    RwListEntry <RwTexture>     TXDList;
    char                        name[RW_TEXTURE_NAME_LENGTH];
    char                        mask[RW_TEXTURE_NAME_LENGTH];
    unsigned int                flags;
    unsigned int                refs;
};
struct RwTextureCoordinates
{
    float u,v;
};
struct RwRaster
{
    RwRaster*       parent;                 // 0
    unsigned char*  pixels;                 // 4
    unsigned char*  palette;                // 8
    int             width, height, depth;   // 12, 16 / 0x10, 20
    int             stride;                 // 24 / 0x18
    short           u, v;
    unsigned char   type;
    unsigned char   flags;
    unsigned char   privateFlags;
    unsigned char   format;
    unsigned char*  origPixels;
    int             origWidth, origHeight, origDepth;
    void*           renderResource;
};
class RwObjectFrame : public RwObject
{
public:
    RwListEntry <RwObjectFrame>     m_lFrame;
    void*                           m_callback;
};
struct RwCameraFrustum
{
    RwPlane       plane;
    unsigned char x,y,z;
    unsigned char unknown1;
};
class RwCamera : public RwObjectFrame
{
public:
    RwCameraType            type;
    RwCameraPreCallback     preCallback;
    RwCameraPostCallback    postCallback;
    RwMatrix                matrix;
    RwRaster*               bufferColor;
    RwRaster*               bufferDepth;
    RwV2d                   screen;
    RwV2d                   screenInverse;
    RwV2d                   screenOffset;
    float                   nearplane;
    float                   farplane;
    float                   fog;
    float                   unknown1;
    float                   unknown2;
    RwCameraFrustum         frustum4D[6];
    RwBBox                  viewBBox;
    RwV3d                   frustum3D[8];
};
class RwRender
{
public:
    unsigned int            m_unknown;
};
struct RpInterpolation
{
    unsigned int     unknown1;
    unsigned int     unknown2;
    float            unknown3;
    float            unknown4;
    float            unknown5;
};

#define RW_ATOMIC_RENDER_REFLECTIVE         0x53F20098
#define RW_ATOMIC_RENDER_VEHICLE            0x53F2009A
#define RW_ATOMIC_RENDER_NIGHT              0x53F2009C

class RpAtomic : public RwObjectFrame
{
public:
    void*                   m_info;             // 20

    RpGeometry*             m_geometry;         // 24
    RwSphere                bsphereLocal;       // 28
    RwSphere                bsphereWorld;       // 44

    RpClump*                m_clump;            // 60
    RwListEntry <RpAtomic>  m_atomics;          // 64

    RpAtomicCallback        m_renderCallback;   // 72
    RpInterpolation         interpolation;      // 76

    unsigned short          frame;              // 96
    unsigned short          unknown7;           // 98
    RwList <void>           sectors;            // 100
    void*                   render;             // 108

    BYTE                    m_pad[8];           // 112
    RpAnimHierarchy*        m_anim;             // 120

    BYTE                    m_pad2[12];         // 124
    unsigned int            m_pipeline;         // 136

    bool                    IsNight();

    void                    SetRenderCallback( RpAtomicCallback callback );
};
struct RpAtomicContainer
{
    RpAtomic    *atomic;
    char        szName[17];
};
class RpLight : public RwObjectFrame
{
public:
    float                   radius;
    RwColorFloat            color;
    float                   unknown1;
    RwList <void>           sectors;
    RwListEntry <RpLight>   globalLights;
    unsigned short          frame;
    unsigned short          unknown2;
};
class RpClump : public RwObject
{   // RenderWare (plugin) Clump (used by GTA)
public:
    RwList <RpAtomic>       m_atomics;          // 8
    RwList <RpLight>        m_lights;           // 16
    RwList <RpCamera>       m_cameras;          // 24
    RwListEntry <RpClump>   m_globalClumps;     // 32
    RpClumpCallback         m_callback;         // 40

    BYTE                    m_pad[16];          // 44

    RwStaticGeometry*       m_static;           // 60

    void                    InitStaticSkeleton();
    RwStaticGeometry*       CreateStaticGeometry();

    RpAnimHierarchy*        GetAtomicAnimHierarchy();
    RpAnimHierarchy*        GetAnimHierarchy();

    RpAtomic*               GetFirstAtomic();
    RpAtomic*               Find2dfx();

    void                    SetupAtomicRender();

    RpClump*                ForAllAtomics( bool (*callback)( RpAtomic *child, void *data ), void *data );

    void                    GetBoneTransform( CVector *offset );
};
struct RpMaterialLighting
{
    float ambient, specular, diffuse;
};
class RpMaterial
{
public:
    RwTexture*          m_texture;
    RwColor             m_color;
    void*               m_render;
    RpMaterialLighting  m_lighting;
    unsigned short      m_refs;
    short               m_id;
    void*               m_unknown;
};
class RpMaterials
{
public:
    RpMaterial**    m_data;
    unsigned int    m_entries;
    unsigned int    m_unknown;
};
struct RpTriangle
{
    unsigned short v1, v2, v3;
    unsigned short materialId;
};
class Rw2dfx
{
public:
    unsigned int            m_count;
};
class RwGeomDimension
{
public:
    BYTE                    m_pad[16];
    float                   m_scale;                            // 16
};
class RpGeometry : public RwObject
{
public:
    unsigned int            flags;                              // 8
    unsigned short          m_unknown1;                         // 12
    unsigned short          m_refs;                             // 14

    unsigned int            m_triangleSize;                     // 16
    unsigned int            m_verticeSize;                      // 20
    unsigned int            m_normalSize;                       // 24
    unsigned int            m_texcoordSize;                     // 28

    RpMaterials             m_materials;                        // 32
    RpTriangle*             m_triangles;                        // 44
    RwColor*                m_colors;                           // 48
    RwTextureCoordinates*   m_texcoords[RW_MAX_TEXTURE_COORDS]; // 52
    void*                   normals;                            // 84
    void*                   info;                               // 88
    RwGeomDimension*        m_dimension;                        // 92
    void*                   m_unknown4;                         // 96
    RpSkeleton*             m_skeleton;                         // 100
    RwColor                 m_nightColor;                       // 104
    BYTE                    m_pad[12];                          // 108
    Rw2dfx                  m_2dfx;                             // 120

    bool                    ForAllMateria( bool (*callback)( RwMaterial *mat, void *data ), void *data );
    bool                    IsAlpha();
};
class RwInterface   // size: 1456
{
public:
    BYTE                    m_pad[304];

    void*                   (*m_malloc)( size_t size );                     // 304
    void                    (*m_free)( void *data );                        // 308
    void*                   (*m_realloc)( void *data, size_t size );        // 312
    void*                   (*m_calloc)( unsigned int count, size_t size ); // 316
    void*                   m_callback;                                     // 320
    void*                   m_callback2;                                    // 324

    BYTE                    m_pad2[28];                                     // 328

    void*                   m_callback3;                                    // 356
    void*                   m_callback4;                                    // 360
    void                    (*m_matrixTransform3)( CVector *dst, const CVector *point, unsigned int count, const RwMatrix *matrices );  // 364
    void*                   m_callback6;                                    // 368

    void*                   m_unknown;                                      // 372
    void*                   m_unknown2;                                     // 376

    void*                   m_callback7;                                    // 380

    BYTE                    m_pad3[36];                                     // 384

    char                    m_charTable[256];                               // 420
    char                    m_charTable2[256];                              // 676

    float                   m_unknown3;                                     // 680
    
    BYTE                    m_pad[172];                                     // 936

    RwRender*               m_renderData;                                   // 1108
    BYTE                    m_pad4[344];                                    // 1112
};

extern RwInterface **ppRwInterface;
#define pRwInterface (*ppRwInterface)

/*****************************************************************************/
/** RenderWare I/O                                                          **/
/*****************************************************************************/

// RenderWare type definitions
typedef int          (* RwIOCallbackClose) (void *data);
typedef unsigned int (* RwIOCallbackRead)  (void *data, void *buffer, unsigned int length);
typedef int          (* RwIOCallbackWrite) (void *data, const void *buffer, unsigned int length);
typedef int          (* RwIOCallbackOther) (void *data, unsigned int offset);

// RenderWare enumerations
enum RwStreamType
{
    STREAM_TYPE_NULL = 0,
    STREAM_TYPE_FILE = 1,
    STREAM_TYPE_FILENAME = 2,
    STREAM_TYPE_BUFFER = 3,
    STREAM_TYPE_CALLBACK = 4,
    STREAM_TYPE_LAST = RW_STRUCT_ALIGN
};
enum RwStreamMode
{
    STREAM_MODE_NULL = 0,
    STREAM_MODE_READ = 1,
    STREAM_MODE_WRITE = 2,
    STREAM_MODE_APPEND = 3,
    STREAM_MODE_LAST = RW_STRUCT_ALIGN
};

// RenderWare base types
struct RwBuffer
{
    void            *ptr;
    unsigned int    size;
};
union RwStreamTypeData
{
    struct {
        unsigned int      position;
        unsigned int      size;
        void              *ptr_file;
    };
    struct {
        void              *file;
    };
    struct {
        RwIOCallbackClose callbackClose;
        RwIOCallbackRead  callbackRead;
        RwIOCallbackWrite callbackWrite;
        RwIOCallbackOther callbackOther;
        void              *ptr_callback;
    };
};
struct RwStream
{
    RwStreamType        type;
    RwStreamMode        mode;
    int                 pos;
    RwStreamTypeData    data;
    int                 id;
};
struct RwError
{
    int err1,err2;
};

#endif
