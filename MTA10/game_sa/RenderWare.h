/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare.h
*  PURPOSE:     RenderWare definitions
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#ifndef __RENDERWARE_COMPAT
#define __RENDERWARE_COMPAT

#include <RenderWare_shared.h>

/*****************************************************************************/
/** RenderWare rendering types                                              **/
/*****************************************************************************/

// RenderWare definitions
#define RW_STRUCT_ALIGN             ((int)((~((unsigned int)0))>>1))
#define RW_TEXTURE_NAME_LENGTH      32
#define RW_MAX_TEXTURE_COORDS       8
#define RW_RENDER_UNIT              215.25f

#define VAR_ATOMIC_RENDER_OFFSET    0x00C88024

// RenderWare extensions
#define RW_EXTENSION_HANIM  0x253F2FB

// Yet to analyze
typedef void   RpWorld;
class RwCamera;
class RpAtomic;
class RpClump;
class RwTexture;
class RpGeometry;

typedef RwCamera*   (*RwCameraPreCallback) (RwCamera * camera);
typedef RwCamera*   (*RwCameraPostCallback) (RwCamera * camera);
typedef bool        (*RpAtomicCallback) (RpAtomic * atomic);
typedef RpClump*    (*RpClumpCallback) (RpClump * clump, void *data);

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

#define RW_OBJ_REGISTERED           0x02
#define RW_OBJ_VISIBLE              0x04
#define RW_OBJ_HIERARCHY_CACHED     0x10

// RenderWare/plugin base types
class RwObject
{
public:
    unsigned char   m_type;
    unsigned char   m_subtype;
    unsigned char   m_flags;
    unsigned char   m_privateFlags;
    class RwFrame*  m_parent;                // should be RwFrame with RpClump

    inline bool         IsVisible()                     { return IS_FLAG( m_flags, RW_OBJ_VISIBLE ); }
    inline void         SetVisible( bool vis )          { BOOL_FLAG( m_flags, RW_OBJ_VISIBLE, vis ); }
};
struct RwVertex
{
    RwV3d        position;
    RwV3d        normal;
    unsigned int color;
    float        u,v;
};

// Macros used by RW, taken from SGU :)
#define LIST_APPEND(link, item) ( (item).next = &(link), (item).prev = (link).prev, (item).prev->next = &(item), (item).next->prev = &(item) )
#define LIST_INSERT(link, item) ( (item).next = (link).next, (item).prev = &(link), (item).prev->next = &(item), (item).next->prev = &(item) )
#define LIST_REMOVE(link) ( (link).prev->next = (link).next, (link).next->prev = (link).prev )
#define LIST_CLEAR(link) ( (link).prev = &(link), (link).next = &(link) )
#define LIST_EMPTY(link) ( (link).prev == &(link) && (link).next == &(link) )
#define LIST_GETITEM(type, item, node) ( (type*)( (unsigned int)(item) - offsetof(type, node) ) )
#define LIST_FOREACH_BEGIN(type, root, node, list, iter) for ( iter = root.next; iter != &root; iter = iter->next ) { type *item = LIST_GETITEM(type, iter, node);
#define LIST_FOREACH_END }

template < class type >
struct RwListEntry
{
    RwListEntry <type> *next, *prev;
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

    RwObject*               m_unknown2;     // 0
    unsigned int            m_unknown;      // 4
    unsigned int            m_count;        // 8
    unsigned int            m_unknown3;     // 12
    RwRenderLink*           m_link;         // 16

    RwRenderLink*           AllocateLink( unsigned int count );
    template <class type>
    void                    ForAllLinks( void (*callback)( RwRenderLink *link, type *data ), type *data )
    {
        RwRenderLink *link = m_link;
        unsigned int n;

        for ( n=0; n<m_count; link++ )
            callback( link, data );
    }
};
class RwBoneInfo
{
public:
    unsigned int            m_index;
    BYTE                    m_pad[4];
    unsigned int            m_flags;
    DWORD                   m_pad2;
};
class RwAnimInfo    // dynamic
{
public:
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
class RpSkeleton
{
public:
    unsigned int            m_boneCount;
    unsigned int            m_unknown;      // 4
    void*                   m_unknown2;     // 8
    RwMatrix*               m_boneMatrices; // 12
    unsigned int            m_unknown3;     // 16
    void*                   m_unknown5;     // 20
    RwV4d*                  m_vertexInfo;   // 24
    unsigned int            m_unknown6;     // 28
    RpAnimation*            m_curAnim;      // 32, ???
    BYTE                    m_pad[28];      // 36
    unsigned char*          m_boneParent2;  // 64
};
class RpAnimHierarchy
{
public:
    unsigned int            m_flags;        // 0
    unsigned int            m_boneCount;    // 4
    char*                   m_data;         // 8
    void*                   m_unknown4;     // 12
    RwBoneInfo*             m_boneInfo;     // 16
    unsigned int            m_unknown5;     // 20
    RpAnimHierarchy*        m_this;         // 24
    unsigned char           m_unknown;      // 28
    unsigned char           m_unknown2;     // 29
    unsigned short          m_unknown3;     // 30
    RpAnimation*            m_anim;         // 32
};
class RwObjectFrame : public RwObject
{
public:
    RwListEntry <RwObjectFrame>     m_lFrame;
    void*                           m_callback;
};
class RwFrame : public RwObject
{
public:
    RwListEntry <RwFrame>   m_nodeRoot;     // 8
    RwMatrix                m_modelling;    // 16
    RwMatrix                m_ltm;          // 80
    RwList <RwObjectFrame>  m_objects;      // 144
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
    template <class type>
    bool                    ForAllChildren( bool (*callback)( RwFrame *frame, type *data ), type *data )
    {
        RwFrame *child;

        for ( child = m_child; child; child = child->m_next )
        {
            if ( !callback( child, data ) )
                return false;
        }

        return true;
    }
    RwFrame*                GetFirstChild();
    RwFrame*                FindFreeChildByName( const char *name );
    RwFrame*                FindChildByName( const char *name );
    RwFrame*                FindChildByHierarchy( unsigned int id );

    template <class type>
    bool                    ForAllObjects( bool (*callback)( RwObject *object, type *data ), type *data )
    {
        RwListEntry <RwObjectFrame> *child;

        for ( child = m_objects.root.next; child != &m_objects.root; child = child->next )
        {
            if ( !callback( (RwObject*)( (unsigned int)child - offsetof(RwObjectFrame, m_lFrame) ), data ) )
                return false;
        }

        return true;
    }
    RwObject*               GetFirstObject();
    RwObject*               GetLastVisibleObject();
    RwObject*               GetLastObject();

    template <class type>
    struct _rwObjectGetAtomic
    {
        bool                (*routine)( RpAtomic *atomic, type data );
        type                data;
    };

    template <class type>
    static bool RwObjectDoAtomic( RwObject *child, _rwObjectGetAtomic <type> *info )
    {
        // Check whether the object is a atomic
        if ( child->m_type != 0x14 )
            return true;

        return info->routine( (RpAtomic*)child, info->data );
    }

    template <class type>
    bool                    ForAllAtomics( bool (*callback)( RpAtomic *atomic, type data ), type data )
    {
        _rwObjectGetAtomic <type> info;

        info.routine = callback;
        info.data = data;

        return ForAllObjects( RwObjectDoAtomic <type>, &info );
    }
    RpAtomic*               GetFirstAtomic();
    void                    SetAtomicVisibility( unsigned short flags );
    void                    BaseAtomicHierarchy();
    void                    FindVisibilityAtomics( RpAtomic **primary, RpAtomic **secondary );

    RpAnimHierarchy*        GetAnimHierarchy();
    void                    RegisterRoot();
};
class RwTexDictionary : public RwObject
{
public:
    RwList <RwTexture>              textures;
    RwListEntry <RwTexDictionary>   globalTXDs;

    template <class type>
    bool                    ForAllTextures( bool (*callback)( RwTexture *tex, type ud ), type ud )
    {
        RwListEntry <RwTexture> *child;

        for ( child = textures.root.next; child != &textures.root; child = child->next )
        {
            if ( !callback( (RwTexture*)( (unsigned int)child - offsetof(RwTexture, TXDList) ), ud ) )
                return false;
        }

        return true;
    }
    RwTexture*              GetFirstTexture();
    RwTexture*              FindNamedTexture( const char *name );
};
class RwRaster
{
public:
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
class RwTexture
{
public:
    RwRaster*                   raster;                         // 0
    RwTexDictionary*            txd;                            // 4
    RwListEntry <RwTexture>     TXDList;                        // 8
    char                        name[RW_TEXTURE_NAME_LENGTH];   // 16
    char                        mask[RW_TEXTURE_NAME_LENGTH];   // 48
    unsigned int                flags;                          // 80
    unsigned int                refs;                           // 84

    void                        AddToDictionary( RwTexDictionary *txd );
    void                        RemoveFromDictionary();
};
struct RwTextureCoordinates
{
    float u,v;
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
struct RpMaterialLighting
{
    float ambient, specular, diffuse;
};
class RpMaterial
{
public:
    RwTexture*          m_texture;      // 0
    RwColor             m_color;        // 4
    void*               m_render;       // 8
    RpMaterialLighting  m_lighting;     // 12
    unsigned short      m_refs;         // 24
    short               m_id;           // 26
    void*               m_unknown;      // 28
};
class RpMaterials
{
public:
                    RpMaterials( unsigned int count );
                    ~RpMaterials();

    bool            Add( RpMaterial *mat );

    RpMaterial**    m_data;
    unsigned int    m_entries;
    unsigned int    m_max;
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

    unsigned char           m_visibility;       // 124
    BYTE                    m_pad2;             // 125
    unsigned char           m_matrixFlags;      // 126
    unsigned char           m_renderFlags;      // 127
    BYTE                    m_pad3[8];          // 128
    unsigned int            m_pipeline;         // 136

    bool                    IsNight();

    void                    SetRenderCallback( RpAtomicCallback callback );

    void                    ApplyVisibilityFlags( unsigned short flags );
    void                    RemoveVisibilityFlags( unsigned short flags );
    unsigned short          GetVisibilityFlags();

    void                    SetExtendedRenderFlags( unsigned short flags );

    void                    FetchMateria( RpMaterials& mats );
};
class RwAtomicZBufferEntry
{
public:
    RpAtomic*               m_atomic;
    RpAtomicCallback        m_render;
    float                   m_distance;
};
class RwAtomicRenderChain
{
public:
    RwAtomicZBufferEntry                m_entry;
    RwListEntry <RwAtomicRenderChain>   list;
};
class RwAtomicRenderChainInterface
{
public:
    RwAtomicRenderChain     m_root;
    RwAtomicRenderChain     m_rootLast;
    RwAtomicRenderChain     m_renderStack;
    RwAtomicRenderChain     m_renderLast;

    bool                    PushRender( RwAtomicZBufferEntry *level );
};

extern RwAtomicRenderChainInterface *rwRenderChains;

struct RpAtomicContainer
{
    RpAtomic*   atomic;
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
    RwList <RwCamera>       m_cameras;          // 24
    RwListEntry <RpClump>   m_globalClumps;     // 32
    RpClumpCallback         m_callback;         // 40

    BYTE                    m_pad[8];           // 44
    unsigned int            m_renderFlags;      // 52
    BYTE                    m_pad2[4];          // 56

    RwStaticGeometry*       m_static;           // 60

    void                    InitStaticSkeleton();
    RwStaticGeometry*       CreateStaticGeometry();

    RpAnimHierarchy*        GetAtomicAnimHierarchy();
    RpAnimHierarchy*        GetAnimHierarchy();

    void                    ScanAtomicHierarchy( RwFrame **atomics, size_t max );

    RpAtomic*               GetFirstAtomic();
    RpAtomic*               Find2dfx();

    void                    SetupAtomicRender();
    void                    RemoveAtomicVisibilityFlags( unsigned short flags );
    void                    FetchMateria( RpMaterials& mats );

    template <class type>
    RpClump*                ForAllAtomics( bool (*callback)( RpAtomic *child, type data ), type data )
    {
        RwListEntry <RpAtomic> *child = m_atomics.root.next;

        for ( ; child != &m_atomics.root; child = child->next )
        {
            if ( !callback( (RpAtomic*)( (unsigned int)child - offsetof(RpAtomic, m_atomics)), data ) )
                return NULL;
        }

        return this;
    }

    void                    GetBoneTransform( CVector *offset );
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
class RwLinkedMaterial
{
    // Appended to RwLinkedMateria per m_count

public:
    void*                   m_unknown;
    unsigned int            m_pad;
    RpMaterial*             m_material;
};
class RwLinkedMateria
{
public:
    unsigned int            m_unknown;
    unsigned short          m_count;
    void*                   m_unknown2;
    BYTE                    m_pad[6];

    RwLinkedMaterial*       Get( unsigned int index );

    // dynamic class
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
    RwLinkedMateria*        m_linkedMateria;                    // 84
    void*                   info;                               // 88
    RwGeomDimension*        m_dimension;                        // 92
    unsigned int            m_usageFlag;                        // 96
    RpSkeleton*             m_skeleton;                         // 100
    RwColor                 m_nightColor;                       // 104
    BYTE                    m_pad[12];                          // 108
    Rw2dfx*                 m_2dfx;                             // 120

    template <class type>
    bool                    ForAllMateria( bool (*callback)( RpMaterial *mat, type data ), type data )
    {
        unsigned int n;

        for ( n=0; n<m_materials.m_entries; n++ )
        {
            if ( !callback( m_materials.m_data[n], data ) )
                return false;
        }

        return true;
    }
    bool                    IsAlpha();
};
class RwStructInfo
{
public:
    size_t                  m_size;
};
class RwInterface   // size: 1456
{
public:
    BYTE                    m_pad[188];

    RwList <RwFrame>        m_nodeRoot;                                     // 188

    BYTE                    m_pad6[112];                                    // 196

    void*                   (*m_malloc)( size_t size );                     // 308
    void                    (*m_free)( void *data );                        // 312
    void*                   (*m_realloc)( void *data, size_t size );        // 316
    void*                   (*m_calloc)( unsigned int count, size_t size ); // 320
    void*                   (*m_allocStruct)( RwStructInfo *info, unsigned int flags ); // 324
    void*                   m_callback2;                                    // 328

    BYTE                    m_pad2[24];                                     // 332

    void*                   m_callback3;                                    // 356
    void*                   m_callback4;                                    // 360
    void                    (*m_matrixTransform3)( CVector *dst, const CVector *point, unsigned int count, const RwMatrix *matrices );  // 364
    void*                   m_callback6;                                    // 368

    void*                   m_unknown;                                      // 372
    RwStructInfo*           m_matrixInfo;                                   // 376

    void*                   m_callback7;                                    // 380

    BYTE                    m_pad3[36];                                     // 384

    char                    m_charTable[256];                               // 420
    char                    m_charTable2[256];                              // 676

    float                   m_unknown3;                                     // 680
    
    BYTE                    m_pad5[172];                                    // 936

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
    void*           ptr;
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
