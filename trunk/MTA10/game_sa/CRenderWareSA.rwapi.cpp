/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.rwapi.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"


/*****************************************************************************
*
*   RenderWare Functions
*
*****************************************************************************/

RwAtomicRenderChainInterface *const rwRenderChains = (RwAtomicRenderChainInterface*)0x00C88070;
RwScene *const *p_gtaScene = (RwScene**)0x00C17038;
RwDeviceInformation *const pRwDeviceInfo = (RwDeviceInformation*)0x00C9BF00;


void RwObjectFrame::AddToFrame( RwFrame *frame )
{
    RemoveFromFrame();

    m_parent = frame;

    if ( !frame )
        return;

    LIST_INSERT( frame->m_objects.root, m_lFrame );
}

void RwObjectFrame::RemoveFromFrame()
{
    if ( !m_parent )
        return;

    LIST_REMOVE( m_lFrame );

    m_parent = NULL;
}

void RwFrame::SetModelling( const RwMatrix& mat )
{
    m_modelling = mat;

    // Set the frame to dirty
    m_privateFlags |= RW_FRAME_DIRTY;
}

void RwFrame::SetPosition( const CVector& pos )
{
    m_modelling.pos = pos;

    // Set the frame to dirty
    m_privateFlags |= RW_FRAME_DIRTY;
}

const RwMatrix& RwFrame::GetLTM() const
{
    // This function will recalculate the LTM if frame is dirty
    return *RwFrameGetLTM( this );
}

void RwFrame::Link( RwFrame *frame )
{
    // Unlink previous relationship of new child
    frame->Unlink();    // interesting side effect: cached if usage of parent

    // Insert the new child at the beginning
    frame->m_next = m_child;
    m_child = frame;

    frame->m_parent = this;

    frame->SetRootForHierarchy( m_root );
    frame->UnregisterRoot();

    // Mark the main root as independent
    m_root->RegisterRoot();
}

void RwFrame::Unlink()
{
    if ( !m_parent )
        return;

    if ( m_parent->m_child == this )
        m_parent->m_child = m_next;
    else
    {
        RwFrame *prev = m_parent->m_child;

        while ( prev->m_next != this )
            prev = prev->m_next;

        prev->m_next = m_next;
    }

    m_parent = NULL;
    m_next = NULL;

    SetRootForHierarchy( this );

    // Mark as independent
    RegisterRoot();
}

void RwFrame::SetRootForHierarchy( RwFrame *root )
{
    m_root = root;

    RwFrame *child = m_child;

    while ( child )
    {
        child->SetRootForHierarchy( root );

        child = child->m_next;
    }
}

static bool RwFrameGetChildCount( RwFrame *child, unsigned int *count )
{
    child->ForAllChildren( RwFrameGetChildCount, count );

    (*count)++;
    return true;
}

unsigned int RwFrame::CountChildren()
{
    unsigned int count = 0;

    ForAllChildren( RwFrameGetChildCount, &count );
    return count;
}

RwFrame* RwFrame::GetFirstChild()
{
    return m_child;
}

struct _rwFrameFindName
{
    const char *name;
    RwFrame *result;
};

static bool RwFrameGetFreeByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( child->m_hierarchyId || strcmp(child->m_nodeName, info->name) != 0 )
        return child->ForAllChildren( RwFrameGetFreeByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindFreeChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetFreeByName, &info ) )
        return NULL;

    return info.result;
}

static bool RwFrameGetByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( stricmp( child->m_nodeName, info->name ) != 0 )
        return child->ForAllChildren( RwFrameGetByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetByName, &info ) )
        return NULL;

    return info.result;
}

struct _rwFrameFindHierarchy
{
    unsigned int    hierarchy;
    RwFrame*        result;
};

static bool RwFrameGetByHierarchy( RwFrame *child, _rwFrameFindHierarchy *info )
{
    if ( child->m_hierarchyId != info->hierarchy )
        return child->ForAllChildren( RwFrameGetByHierarchy, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByHierarchy( unsigned int id )
{
    _rwFrameFindHierarchy info;

    info.hierarchy = id;

    if ( ForAllChildren( RwFrameGetByHierarchy, &info ) )
        return NULL;

    return info.result;
}

RwFrame* RwFrame::CloneRecursive() const
{
    RwFrame *cloned = RwFrameCloneRecursive( this, NULL );

    if ( !cloned )
        return NULL;

    cloned->m_privateFlags &= ~( RW_OBJ_REGISTERED | RW_FRAME_DIRTY );
    cloned->RegisterRoot();
    return cloned;
}

static bool RwFrameGetAnimHierarchy( RwFrame *frame, RpAnimHierarchy **rslt )
{
    if ( frame->m_anim )
    {
        *rslt = frame->m_anim;
        return false;
    }

    return frame->ForAllChildren( RwFrameGetAnimHierarchy, rslt );
}

static bool RwObjectGet( RwObject *child, RwObject **dst )
{
    *dst = child;
    return false;
}

RwObject* RwFrame::GetFirstObject()
{
    RwObject *obj;

    if ( ForAllObjects( RwObjectGet, &obj ) )
        return NULL;

    return obj;
}

struct _rwFindObjectType
{
    unsigned char type;
    RwObject *rslt;
};

static bool RwObjectGetByType( RwObject *child, _rwFindObjectType *info )
{
    if ( child->m_type != info->type )
        return true;

    info->rslt = child;
    return false;
}

RwObject* RwFrame::GetFirstObject( unsigned char type )
{
    _rwFindObjectType info;
    info.type = type;

    return ForAllObjects( RwObjectGetByType, &info ) ? NULL : info.rslt;
}

struct _rwObjectByIndex
{
    unsigned char type;
    unsigned int idx;
    unsigned int curIdx;
    RwObject *rslt;
};

static bool RwObjectGetByIndex( RwObject *obj, _rwObjectByIndex *info )
{
    if ( obj->m_type != info->type )
        return true;

    if ( info->idx != info->curIdx )
    {
        info->curIdx++;
        return true;
    }

    info->rslt = obj;
    return false;
}

RwObject* RwFrame::GetObjectByIndex( unsigned char type, unsigned int idx )
{
    _rwObjectByIndex info;
    info.type = type;
    info.idx = idx;
    info.curIdx = 0;
    
    if ( ForAllObjects( RwObjectGetByIndex, &info ) )
        return NULL;

    return info.rslt;
}

struct _rwObjCntByType
{
    unsigned char type;
    unsigned int cnt;
};

static bool RwFrameCountObjectsByType( RwObject *obj, _rwObjCntByType *info )
{
    if ( obj->m_type == info->type )
        info->cnt++;

    return true;
}

unsigned int RwFrame::CountObjectsByType( unsigned char type )
{
    _rwObjCntByType info;
    info.type = type;
    info.cnt = 0;

    ForAllObjects( RwFrameCountObjectsByType, &info );
    return info.cnt;
}

static bool RwFrameObjectGetLast( RwObject *obj, RwObject **dst )
{
    *dst = obj;
    return true;
}

RwObject* RwFrame::GetLastObject()
{
    RwObject *obj = NULL;

    ForAllObjects( RwFrameObjectGetLast, &obj );
    return obj;
}

static bool RwFrameObjectGetVisibleLast( RwObject *obj, RwObject **dst )
{
    if ( obj->m_flags & RW_OBJ_VISIBLE )
        *dst = obj;

    return true;
}

RwObject* RwFrame::GetLastVisibleObject()
{
    RwObject *obj = NULL;

    ForAllObjects( RwFrameObjectGetVisibleLast, &obj );
    return obj;
}

static bool RwObjectGetAtomic( RpAtomic *atomic, RpAtomic **dst )
{
    *dst = atomic;
    return false;
}

RpAtomic* RwFrame::GetFirstAtomic()
{
    RpAtomic *atomic;

    if ( ForAllAtomics( RwObjectGetAtomic, &atomic ) )
        return NULL;

    return atomic;
}

static bool RwObjectAtomicSetVisibilityFlags( RpAtomic *atomic, void *ud )
{
    atomic->ApplyVisibilityFlags( (unsigned short)ud );
    return true;
}

void RwFrame::SetAtomicVisibility( unsigned short flags )
{
    ForAllAtomics( RwObjectAtomicSetVisibilityFlags, (void*)flags );
}

static bool RwFrameAtomicBaseRoot( RpAtomic *atomic, RwFrame *root )
{
    RpAtomicSetFrame( atomic, root );
    return true;
}

struct _rwFrameVisibilityAtomics
{
    RpAtomic **primary;
    RpAtomic **secondary;
};

static bool RwFrameAtomicFindVisibility( RpAtomic *atomic, _rwFrameVisibilityAtomics *info )
{
    if ( atomic->GetVisibilityFlags() & 0x01 )
    {
        *info->primary = atomic;
        return true;
    }

    if ( atomic->GetVisibilityFlags() & 0x02 )
        *info->secondary = atomic;

    return true;
}

void RwFrame::FindVisibilityAtomics( RpAtomic **primary, RpAtomic **secondary )
{
    _rwFrameVisibilityAtomics info;

    info.primary = primary;
    info.secondary = secondary;

    ForAllAtomics( RwFrameAtomicFindVisibility, &info );
}

RpAnimHierarchy* RwFrame::GetAnimHierarchy()
{
    RpAnimHierarchy *anim;

    if ( m_anim )
        return m_anim;

    // We want false, since it has to interrupt == found
    if ( ForAllChildren( RwFrameGetAnimHierarchy, &anim ) )
        return NULL;

    return anim;
}

void RwFrame::RegisterRoot()
{
    if ( !(m_root->m_privateFlags & ( RW_OBJ_REGISTERED | RW_FRAME_DIRTY ) ) )
    {
        // Add it to the internal list
        LIST_INSERT( pRwInterface->m_nodeRoot.root, m_nodeRoot );

        m_root->m_privateFlags |= RW_OBJ_REGISTERED | RW_FRAME_DIRTY;
    }

    m_privateFlags |= RW_OBJ_REGISTERED | RW_OBJ_HIERARCHY_CACHED;
}

void RwFrame::UnregisterRoot()
{
    if ( !(m_root->m_privateFlags & ( RW_OBJ_REGISTERED | 0x01 ) ) )
        return;

    LIST_REMOVE( m_nodeRoot );

    m_privateFlags &= ~(RW_OBJ_REGISTERED | 1);
}

static bool RwTexDictionaryGetFirstTexture( RwTexture *tex, RwTexture **rslt )
{
    *rslt = tex;
    return false;
}

RwTexture* RwTexDictionary::GetFirstTexture()
{
    RwTexture *tex;

    if ( ForAllTextures( RwTexDictionaryGetFirstTexture, &tex ) )
        return NULL;

    return tex;
}

struct _rwTexDictFind
{
    const char *name;
    RwTexture *tex;
};

static bool RwTexDictionaryFindTexture( RwTexture *tex, _rwTexDictFind *find )
{
    if ( stricmp( tex->name, find->name ) != 0 )
        return true;

    find->tex = tex;
    return false;
}

RwTexture* RwTexDictionary::FindNamedTexture( const char *name )
{
    _rwTexDictFind find;
    find.name = name;

    if ( ForAllTextures( RwTexDictionaryFindTexture, &find ) )
        return NULL;

    return find.tex;
}

void RwTexture::AddToDictionary( RwTexDictionary *_txd )
{
    if ( txd )
        LIST_REMOVE( TXDList );

    LIST_INSERT( _txd->textures.root, TXDList );

    txd = _txd;
}

void RwTexture::RemoveFromDictionary()
{
    if ( !txd )
        return;

    LIST_REMOVE( TXDList );

    txd = NULL;
}

RwCamera* RwCameraCreate()
{
    RwCamera *cam = (RwCamera*)pRwInterface->m_allocStruct( pRwInterface->m_cameraInfo, 0x30005 );

    if ( !cam )
        return NULL;

    cam->m_type = RW_CAMERA;
    cam->m_subtype = 0;
    cam->m_flags = 0;
    cam->m_privateFlags = 0;
    cam->m_parent = NULL;

    cam->m_callback = (void*)0x007EE5A0;
    cam->m_preCallback = (RwCameraPreCallback)0x007EF370;
    cam->m_postCallback = (RwCameraPostCallback)0x007EF340;

    cam->m_screen.x = cam->m_screen.y = cam->m_screenInverse.x = cam->m_screenInverse.y = 1;
    cam->m_screenOffset.x = cam->m_screenOffset.y = 0;

    cam->m_nearplane = 0.05f;
    cam->m_farplane = 10;
    cam->m_fog = 5;

    cam->m_rendertarget = NULL;
    cam->m_bufferDepth = NULL;
    cam->m_camType = RW_CAMERA_PERSPECTIVE;

    // Do some plane shifting
    ((void (__cdecl*)( RwCamera* ))0x007EE200)( cam );

    cam->m_matrix.a = 0;

    RwObjectRegister( (void*)0x008E222C, cam );
    return cam;
}

void RwCamera::BeginUpdate()
{
    m_preCallback( this );
}

void RwCamera::EndUpdate()
{
    m_postCallback( this );
}

void RwCamera::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->m_cameras.root, m_clumpCameras );

    m_clump = clump;
}

void RwCamera::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_clumpCameras );

    m_clump = NULL;
}

RwStaticGeometry::RwStaticGeometry()
{
    m_count = 0;
    m_link = NULL;
}

RwRenderLink* RwStaticGeometry::AllocateLink( unsigned int count )
{
    if ( m_link )
        RwFreeAligned( m_link );

    m_count = count;
    return m_link = (RwRenderLink*)RwAllocAligned( (((count * sizeof(RwRenderLink) - 1) >> 6 ) + 1) << 6, 0x40 );
}

bool RpAtomic::IsNight()
{
    if ( m_pipeline == RW_ATOMIC_RENDER_NIGHT )
        return true;

    if ( m_pipeline == RW_ATOMIC_RENDER_REFLECTIVE )
        return false;

    return m_geometry->m_nightColor && m_geometry->m_colors != NULL;
}

void RpAtomic::AddToClump( RpClump *clump )
{
    RemoveFromClump();

    m_clump = clump;

    LIST_INSERT( clump->m_atomics.root, m_atomics );
}

void RpAtomic::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_atomics );

    m_clump = NULL;
}

void RpAtomic::SetRenderCallback( RpAtomicCallback callback )
{
    if ( !callback )
    {
        m_renderCallback = (RpAtomicCallback)RpAtomicRender;
        return;
    }

    m_renderCallback = callback;
}

void RpAtomic::ApplyVisibilityFlags( unsigned short flags )
{
    *(unsigned short*)&m_matrixFlags |= flags;
}

void RpAtomic::RemoveVisibilityFlags( unsigned short flags )
{
    *(unsigned short*)&m_matrixFlags &= ~flags;
}

unsigned short RpAtomic::GetVisibilityFlags()
{
    return *(unsigned short*)&m_matrixFlags;
}

void RpAtomic::SetExtendedRenderFlags( unsigned short flags )
{
    *(unsigned short*)&m_renderFlags = flags;
}

void RpAtomic::FetchMateria( RpMaterials& mats )
{
    unsigned int n;

    if ( !(GetVisibilityFlags() & 0x20) )
        return;

    for (n=0; n<m_geometry->m_linkedMateria->m_count; n++)
        mats.Add( m_geometry->m_linkedMateria->Get(n)->m_material );
}

RpMaterials::RpMaterials( unsigned int max )
{
    m_data = (RpMaterial**)pRwInterface->m_malloc( sizeof(long) * max );

    m_max = max;
    m_entries = 0;
}

RpMaterials::~RpMaterials()
{
    unsigned int n;

    for ( n=0; n<m_entries; n++ )
        RpMaterialDestroy( m_data[n] );

    pRwInterface->m_free( m_data );

    m_entries = 0;
    m_max = 0;
}

bool RpMaterials::Add( RpMaterial *mat )
{
    if ( m_entries == m_max )
        return false;   // We do not need RW error handlers anymore?

    // Reference it
    mat->m_refs++;

    m_data[ m_entries++ ] = mat;
    return true;
}

RwLinkedMaterial* RwLinkedMateria::Get( unsigned int index )
{
    if ( index >= m_count )
        return NULL;

    return (RwLinkedMaterial*)(this + 1) + index;
}

void RpLight::SetLightIndex( unsigned int idx )
{
    m_lightIndex = min( idx, 8 );
}

void RpLight::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->m_lights.root, m_clumpLights );

    m_clump = clump;
}

void RpLight::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_clumpLights );

    m_clump = NULL;
}

void RpLight::AddToScene_Global( RwScene *scene )
{
    RemoveFromScene();

    m_scene = scene;

    LIST_INSERT( scene->m_globalLights.root, m_sceneLights );
}

void RpLight::AddToScene_Local( RwScene *scene )
{
    RemoveFromScene();

    m_scene = scene;

    if ( scene->m_parent )
        scene->m_parent->RegisterRoot();

    LIST_INSERT( scene->m_localLights.root, m_sceneLights );
}

void RpLight::AddToScene( RwScene *scene )
{
    if ( m_subtype < 0x80 )
        AddToScene_Global( scene );
    else
        AddToScene_Local( scene );
}

void RpLight::RemoveFromScene()
{
    if ( !m_scene )
        return;

    RwSceneRemoveLight( m_scene, this );
}

void RpClump::Render()
{
    LIST_FOREACH_BEGIN( RpAtomic, m_atomics.root, m_atomics )
        if ( item->IsVisible() )
        {
            item->m_parent->GetLTM();   // Possibly update it's world position
            item->m_renderCallback( item );
        }
    LIST_FOREACH_END
}

static bool RwAssignRenderLink( RwFrame *child, RwRenderLink **link )
{
    (*link)->m_context = child;
    (*link)++;

    child->ForAllChildren( RwAssignRenderLink, link );
    return true;
}

static void RwRenderLinkInit( RwRenderLink *link, void *data )
{
    RwFrame *frame = (RwFrame*)link->m_context;

    link->m_flags = 0;

    link->m_position = frame->GetPosition();

    link->m_id = -1;
}

static void RwAnimatedRenderLinkInit( RwRenderLink *link, int )
{
    link->m_flags = 0;
}

void RpClump::InitStaticSkeleton()
{
    RpAtomic *atomic = GetFirstAtomic();
    RwStaticGeometry *geom = CreateStaticGeometry();
    RwRenderLink *link;
    CVector boneOffsets[MAX_BONES];
    unsigned int boneCount;

    if ( !atomic || !atomic->m_geometry->m_skeleton )
    {
        // Initialize a non animated mesh
        link = geom->AllocateLink( m_parent->CountChildren() );

        // Assign all frames
        m_parent->ForAllChildren( RwAssignRenderLink, &link );

        // Init them
        geom->ForAllLinks( RwRenderLinkInit, (void*)NULL );

        geom->m_link->m_flags |= BONE_ROOT;
        return;
    }

    // Grab the number of bones
    boneCount = atomic->m_geometry->m_skeleton->m_boneCount;

    link = geom->AllocateLink( boneCount );

    if ( boneCount != 0 )
    {
        CVector *offset = boneOffsets;
        RpAnimHierarchy *anim = atomic->m_anim;  
        RwBoneInfo *bone = anim->m_boneInfo;
        RwAnimInfo *info = anim->m_anim->m_info;
        unsigned int n;

        // Get the real bone positions
        GetBoneTransform( boneOffsets );

        // I guess its always one bone ahead...?
        link++;

        for (n=0; n<boneCount; n++)
        {
            link->m_context = info;
            link->m_id = bone->m_index;

            // Update the bone offset in the animation
            info->m_offset = *offset;
            
            info++;
            offset++;
            link++;
            bone++;
        }
    }

    geom->ForAllLinks( RwAnimatedRenderLinkInit, 0 );

    // Flag the first render link, root bone?
    geom->m_link->m_flags |= BONE_ROOT;
}

RwStaticGeometry* RpClump::CreateStaticGeometry()
{
    return m_static = new RwStaticGeometry();
}

RpAnimHierarchy* RpClump::GetAtomicAnimHierarchy()
{
    RpAtomic *atomic = GetFirstAtomic();

    if (!atomic)
        return NULL;
    
    return atomic->m_anim;
}

RpAnimHierarchy* RpClump::GetAnimHierarchy()
{
    return m_parent->GetAnimHierarchy();
}

struct _rwFrameScanHierarchy
{
    RwFrame **output;
    size_t max;
};

static bool RwFrameGetAssignedHierarchy( RwFrame *child, _rwFrameScanHierarchy *info )
{
    if ( child->m_hierarchyId && child->m_hierarchyId < info->max )
        info->output[child->m_hierarchyId] = child;

    return child->ForAllChildren( RwFrameGetAssignedHierarchy, info );
}

void RpClump::ScanAtomicHierarchy( RwFrame **atomics, size_t max )
{
    _rwFrameScanHierarchy info;

    info.output = atomics;
    info.max = max;

    m_parent->ForAllChildren( RwFrameGetAssignedHierarchy, &info );
}

RpAtomic* RpClump::GetFirstAtomic()
{
    if ( LIST_EMPTY( m_atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, m_atomics.root.next, m_atomics );
}

RpAtomic* RpClump::GetLastAtomic()
{
    if ( LIST_EMPTY( m_atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, m_atomics.root.prev, m_atomics );
}

struct _rwFindAtomicNamed
{
    const char *name;
    RpAtomic *rslt;
};

static bool RpClumpFindNamedAtomic( RpAtomic *atom, _rwFindAtomicNamed *info )
{
    if ( strcmp( atom->m_parent->m_nodeName, info->name ) != 0 )
        return true;

    info->rslt = atom;
    return false;
}

RpAtomic* RpClump::FindNamedAtomic( const char *name )
{
    _rwFindAtomicNamed info;
    info.name = name;

    return ForAllAtomics( RpClumpFindNamedAtomic, &info ) ? NULL : info.rslt;
}

static bool RwAtomicGet2dfx( RpAtomic *child, RpAtomic **atomic )
{
    // Crashfix, invalid geometry
    if ( !child->m_geometry )
        return true;

    if ( !child->m_geometry->m_2dfx || child->m_geometry->m_2dfx->m_count == 0 )
        return true;

    *atomic = child;
    return false;
}

RpAtomic* RpClump::Find2dfx()
{
    RpAtomic *atomic;

    if ( ForAllAtomics( RwAtomicGet2dfx, &atomic ) )
        return NULL;

    return atomic;
}

static bool RwAtomicSetupPipeline( RpAtomic *child, int )
{
    if ( child->IsNight() )
        RpAtomicSetupObjectPipeline( child );
    else if ( child->m_pipeline == RW_ATOMIC_RENDER_VEHICLE )
        RpAtomicSetupVehiclePipeline( child );

    return true;
}

void RpClump::SetupAtomicRender()
{
    ForAllAtomics( RwAtomicSetupPipeline, 0 );
}

static bool RwAtomicRemoveVisibilityFlags( RpAtomic *child, unsigned short flags )
{
    child->RemoveVisibilityFlags( flags );
    return true;
}

void RpClump::RemoveAtomicVisibilityFlags( unsigned short flags )
{
    ForAllAtomics( RwAtomicRemoveVisibilityFlags, flags );
}

static bool RwAtomicFetchMateria( RpAtomic *child, RpMaterials *mats )
{
    child->FetchMateria( *mats );
    return true;
}

void RpClump::FetchMateria( RpMaterials& mats )
{
    ForAllAtomics( RwAtomicFetchMateria, &mats );
}

void RpClump::GetBoneTransform( CVector *offset )
{
    RpAtomic *atomic;
    RpSkeleton *skel;
    RpAnimHierarchy* anim;
    RwBoneInfo *bone;
    RwMatrix *skelMat;
    unsigned int n;
    unsigned int matId = 0;
    unsigned int boneIndexes[20];
    unsigned int *idxPtr = boneIndexes;

    if ( !offset )
        return;

    atomic = GetFirstAtomic();
    skel = atomic->m_geometry->m_skeleton;

    anim = atomic->m_anim;

    // Reset the matrix
    offset->fX = 0;
    offset->fY = 0;
    offset->fZ = 0;

    offset++;

    // Do nothing if the bone count is smaller than 2
    if ( skel->m_boneCount < 2 )
        return;

    skelMat = skel->m_boneMatrices + 1;
    bone = anim->m_boneInfo + 1;

    // We apparrently have the first one initialized already?
    for (n=1; n<skel->m_boneCount; n++)
    {
        RwMatrix mat;

        RwMatrixInvert( &mat, skelMat );

        pRwInterface->m_matrixTransform3( offset, &mat.up, 1, skel->m_boneMatrices + matId );

        // Some sort of stacking mechanism, maximum 20
        if ( bone->m_flags & 0x02 )
            *(++idxPtr) = matId;

        if ( bone->m_flags & 0x01 )
            matId = *(idxPtr--);
        else
            matId = n;

        skelMat++;
        offset++;
        bone++;
    }
}

bool RwMaterialAlphaCheck( RpMaterial *mat, int )
{
    return mat->m_color.a != 0xFF;
}

bool RpGeometry::IsAlpha()
{
    return !ForAllMateria( RwMaterialAlphaCheck, 0 );
}

bool RpMaterialTextureUnlink( RpMaterial *mat, int )
{
    if ( RwTexture *tex = mat->m_texture )
        mat->m_texture = NULL;

    return true;
}

void RpGeometry::UnlinkFX()
{
    // Clean all texture links
    ForAllMateria( RpMaterialTextureUnlink, 0 );

    if ( m_2dfx )
    {
        // Clean the 2dfx structure
        pRwInterface->m_free( m_2dfx );
        m_2dfx = NULL;
    }
}

bool RwAtomicRenderChainInterface::PushRender( RwAtomicZBufferEntry *level )
{
    RwAtomicRenderChain *iter = &m_root;
    RwAtomicRenderChain *progr;

    while ( iter->list.prev != &m_rootLast.list )
    {
        iter = LIST_GETITEM(RwAtomicRenderChain, iter->list.prev, list);

        if ( iter->m_entry.m_distance >= level->m_distance )
            break;
    }

    if ( ( progr = LIST_GETITEM(RwAtomicRenderChain, m_renderStack.list.prev, list) ) == &m_renderLast )
        return false;

    // Update render details
    progr->m_entry = *level;

    LIST_REMOVE( progr->list );
    LIST_INSERT( iter->list, progr->list );
    return true;
}

RwTexture* RwFindTexture( const char *name, const char *secName )
{
    RwTexture *tex = pRwInterface->m_textureManager.m_findInstanceRef( name );

    // The global store will reference textures
    if ( tex )
    {
        tex->refs++;
        return tex;
    }

    if ( !( tex = pRwInterface->m_textureManager.m_findInstance( name, secName ) ) )
    {
        // If we have not found anything, we tell the system about an error
        RwError err;
        err.err1 = 0x01;
        err.err2 = 0x16;

        // Actually, there is a missing texture handler; it is void though
        RwSetError( &err );
        return NULL;
    }

    if ( RwTexDictionary *txd = pRwInterface->m_textureManager.m_current )
    {
        tex->RemoveFromDictionary();
        tex->AddToDictionary( txd );
    }

    return tex;
}

RwError* RwSetError( RwError *info )
{
    if ( pRwInterface->m_errorInfo.err1 )
        return info;

    if ( pRwInterface->m_errorInfo.err2 != 0x80000000 )
        return info;

    if ( info->err1 & 0x80000000 )
        pRwInterface->m_errorInfo.err1 = 0;
    else
        pRwInterface->m_errorInfo.err1 = info->err1;

    pRwInterface->m_errorInfo.err2 = info->err2;
    return info;
}

static void* _lightCallback( void *ptr )
{
    return ptr;
}

RpLight* RpLightCreate( unsigned char type )
{
    RpLight *light = (RpLight*)pRwInterface->m_allocStruct( pRwInterface->m_lightInfo, 0x30012 );

    if ( !light )
        return NULL;

    light->m_type = RW_LIGHT;
    light->m_subtype = type;
    light->m_color.a = 0;
    light->m_color.r = 0;
    light->m_color.g = 0;
    light->m_color.b = 0;

    light->m_callback = (void*)_lightCallback;
    light->m_flags = 0;
    light->m_parent = NULL;

    light->m_radius = 0;
    light->m_coneAngle = 0;
    light->m_privateFlags = 0;

    // Clear the list awareness
    LIST_CLEAR( light->m_sectors.root );
    LIST_INITNODE( light->m_clumpLights );

    light->m_flags = 3; // why write it again? R* hack?
    light->m_frame = pRwInterface->m_frame;

    RwObjectRegister( (void*)0x008D62F8, light );
    return light;
}

void RpLight::SetColor( const RwColorFloat& color )
{
    m_color = color;
    
    // Check whether we are brightness only
    m_privateFlags = ( m_color.r == m_color.g && m_color.r == m_color.b );
}

static RpClump* _clumpCallback( RpClump *clump, void *data )
{
    return clump;
}

RpClump* RpClumpCreate()
{
    RpClump *clump = (RpClump*)pRwInterface->m_allocStruct( pRwInterface->m_clumpInfo, 0x30010 );

    if ( !clump )
        return NULL;

    clump->m_type = RW_CLUMP;

    LIST_CLEAR( clump->m_atomics.root );
    LIST_CLEAR( clump->m_lights.root );
    LIST_CLEAR( clump->m_cameras.root );

    clump->m_subtype = 0;
    clump->m_flags = 0;
    clump->m_privateFlags = 0;
    clump->m_parent = NULL;

    LIST_INITNODE( clump->m_globalClumps );

    clump->m_callback = _clumpCallback;

    RwObjectRegister( (void*)0x008D6264, clump );
    return clump;
}