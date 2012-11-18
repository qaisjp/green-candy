/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/gamesa_renderware.h
*  PURPOSE:     RenderWare interface mappings to Grand Theft Auto: San Andreas
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#ifndef __GAMESA_RENDERWARE
#define __GAMESA_RENDERWARE

#define WIN32_LEAN_AND_MEAN

#include "RenderWare.h"

class CColModelSAInterface;

/*****************************************************************************/
/** Renderware functions                                                    **/
/*****************************************************************************/

/* RenderWare function defines */
typedef void                    (__cdecl *RwRenderSystemFigureAffairs_t)        (RwRenderSystem& rend, int objectID, unsigned int& result, int, int);
typedef void*                   (__cdecl *RwAllocAligned_t)                     (size_t size, unsigned int align);
typedef void                    (__cdecl *RwFreeAligned_t)                      (void *ptr);
typedef RwExtension*            (__cdecl *RwCreateExtension_t)                  (unsigned int id, unsigned int count, size_t size, int unk3);
typedef void                    (__cdecl *RwObjectRegister_t)                   (void *group, RwObject *obj);
typedef RwStream*               (__cdecl *RwStreamOpen_t)                       (RwStreamType type, RwStreamMode mode, const void *pData);
typedef int                     (__cdecl *RwStreamFindChunk_t)                  (RwStream *stream, unsigned int type, unsigned int *lengthOut, unsigned int *versionOut);
typedef unsigned int            (__cdecl *RwStreamReadBlocks_t)                 (RwStream *stream, unsigned int *numBlocks, unsigned int size);
typedef RwTexture*              (__cdecl *RwStreamReadTexture_t)                (RwStream *stream);
typedef int                     (__cdecl *RwStreamClose_t)                      (RwStream *stream, void *pData);
typedef RpAtomic*               (__cdecl *RpAtomicCreate_t)                     ();
typedef RpAtomic*               (__cdecl *RpAtomicClone_t)                      (const RpAtomic *atomic);
typedef RpAtomic*               (__cdecl *RpAtomicSetGeometry_t)                (RpAtomic *atomic, RpGeometry *geometry, unsigned int flags);
typedef RpAtomic*               (__cdecl *RpAtomicSetFrame_t)                   (RpAtomic *atomic, RwFrame *frame);
typedef void                    (__cdecl *RpAtomicSetupObjectPipeline_t)        (RpAtomic *atomic);
typedef void                    (__cdecl *RpAtomicSetupVehiclePipeline_t)       (RpAtomic *atomic);
typedef bool                    (__cdecl *RpAtomicRender_t)                     (RpAtomic *atomic);
typedef bool                    (__cdecl *RpAtomicRenderEx_t)                   (RpAtomic *atomic, unsigned int flags);
typedef int                     (__cdecl *RpAtomicDestroy_t)                    (RpAtomic *atomic);
typedef RwCamera*               (__cdecl *RwCameraClone_t)                      (RwCamera *camera);
typedef void                    (__cdecl *RwCameraDestroy_t)                    (RwCamera *camera);
typedef RpClump*                (__cdecl *RpClumpClone_t)                       (const RpClump *clone);
typedef RpClump*                (__cdecl *RpClumpStreamRead_t)                  (RwStream *stream);
typedef RpClump*                (__cdecl *RpClumpAddAtomic_t)                   (RpClump *clump, RpAtomic *atomic);
typedef RpClump*                (__cdecl *RpClumpAddLight_t)                    (RpClump *clump, RpLight *light);
typedef int                     (__cdecl *RpClumpGetNumAtomics_t)               (RpClump *clump);
typedef RpClump*                (__cdecl *RpClumpRemoveAtomic_t)                (RpClump *clump, RpAtomic *atomic);
typedef void                    (__cdecl *RpClumpGetBoneTransform_t)            (RpClump *clump, CVector *offsets);
typedef void                    (__cdecl *RpClumpSetupFrameCallback_t)          (RpClump *clump, unsigned int hierarchyId);
typedef void                    (__cdecl *RpClumpRender_t)                      (RpClump *clump);
typedef int                     (__cdecl *RpClumpDestroy_t)                     (RpClump *clump);
typedef bool                    (__cdecl *RwAnimationInit_t)                    (RpAnimation *anim, RwExtension *ext);
typedef bool                    (__cdecl *RwSkeletonUpdate_t)                   (RpSkeleton *skel);
typedef RwFrame*                (__cdecl *RwFrameCreate_t)                      ();
typedef const RwMatrix*         (__cdecl *RwFrameGetLTM_t)                      (const RwFrame *frame);
typedef RwFrame*                (__cdecl *RwFrameAddChild_t)                    (RwFrame *parent, RwFrame *child);
typedef RwFrame*                (__cdecl *RwFrameRemoveChild_t)                 (RwFrame *child);
typedef RwFrame*                (__cdecl *RwFrameTranslate_t)                   (RwFrame *frame, const RwV3d *v, RwTransformOrder order);
typedef RwFrame*                (__cdecl *RwFrameScale_t)                       (RwFrame *frame, const RwV3d *v, RwTransformOrder order);
typedef void                    (__cdecl *RwFrameOrient_t)                      (RwFrame *frame, float unk, float unk2, CVector& unk3);
typedef RwFrame*                (__cdecl *RwFrameSetIdentity_t)                 (RwFrame *frame);
typedef void                    (__cdecl *RwFrameCloneHierarchy_t)              (RwFrame *frame);
typedef void                    (__cdecl *RwFrameDestroy_t)                     (RwFrame *frame);
typedef void                    (__cdecl *RwObjectFrameRender_t)                (RwRender *data, RwObjectFrame *frame, unsigned int unk);
typedef RpGeometry*             (__cdecl *RpGeometryCreate_t)                   (int numverts, int numtriangles, unsigned int format);
typedef void                    (__cdecl *RpGeometryAddRef_t)                   (RpGeometry *geom);
typedef RpAnimHierarchy*        (__cdecl *RpGeometryGetAnimation_t)             (RpGeometry *geom);
typedef const RpGeometry*       (__cdecl *RpGeometryTriangleSetVertexIndices_t) (const RpGeometry *geo, RpTriangle *tri, unsigned short v1, unsigned short v2, unsigned short v3);
typedef RpGeometry*             (__cdecl *RpGeometryUnlock_t)                   (RpGeometry *geo);
typedef RpGeometry*             (__cdecl *RpGeometryLock_t)                     (RpGeometry *geo, int lockmode);
typedef RpGeometry*             (__cdecl *RpGeometryTransform_t)                (RpGeometry *geo, const RwMatrix *matrix);
typedef RpGeometry*             (__cdecl *RpGeometryTriangleSetMaterial_t)      (RpGeometry *geo, RpTriangle *tri, RpMaterial *mat);
typedef int                     (__cdecl *RpGeometryDestroy_t)                  (RpGeometry *geo);
typedef void*                   (__cdecl *RwIm3DTransform_t)                    (RwVertex *pVerts, unsigned int numVerts, RwMatrix *ltm, unsigned int flags);
typedef int                     (__cdecl *RwIm3DRenderIndexedPrimitive_t)       (RwPrimitiveType primType, unsigned short *indices, int numIndices);
typedef int                     (__cdecl *RwIm3DEnd_t)                          ();
typedef RpLight*                (__cdecl *RpLightSetRadius_t)                   (RpLight *light, float radius);
typedef void                    (__cdecl *RpLightDestroy_t)                     (RpLight *light);
typedef RwMatrix*               (__cdecl *RwMatrixCreate_t)                     ();
typedef RwMatrix*               (__cdecl *RwMatrixInvert_t)                     (RwMatrix *dst, const RwMatrix *src);
typedef RwMatrix*               (__cdecl *RwMatrixTranslate_t)                  (RwMatrix *matrix, const RwV3d *translation, RwTransformOrder order);
typedef RwMatrix*               (__cdecl *RwMatrixScale_t)                      (RwMatrix *matrix, const RwV3d *translation, RwTransformOrder order);
typedef float                   (__cdecl *RwMatrixUnknown_t)                    (const RwMatrix& matrix, const RwMatrix& matrix2, unsigned char flags);
typedef RpMaterial*             (__cdecl *RpMaterialCreate_t)                   ();
typedef int                     (__cdecl *RpMaterialDestroy_t)                  (RpMaterial *mat);
typedef RwTexDictionary*        (__cdecl *RwTexDictionaryCreate_t)              ();
typedef unsigned int            (__cdecl *RwTexDictionaryFinalizer_t)           (void *unk, RwStream *stream, RwTexDictionary *txd);
typedef RwTexDictionary*        (__cdecl *RwTexDictionaryStreamRead_t)          (RwStream *stream);
typedef RwTexDictionary*        (__cdecl *RwTexDictionarySetCurrent_t)          (RwTexDictionary *dict);
typedef const RwTexDictionary*  (__cdecl *RwTexDictionaryForAllTextures_t)      (const RwTexDictionary *dict, int (*callback)( RwTexture *texture, void *data ), void *data);
typedef RwTexture*              (__cdecl *RwTexDictionaryAddTexture_t)          (RwTexDictionary *dict, RwTexture *texture);
typedef RwTexDictionary*        (__cdecl *RwTexDictionaryGetCurrent_t)          ();
typedef RwTexture*              (__cdecl *RwTexDictionaryFindNamedTexture_t)    (RwTexDictionary *dict, const char* name);
typedef int                     (__cdecl *RwTexDictionaryDestroy_t)             (RwTexDictionary *txd);
typedef RwTexture*              (__cdecl *RwTextureCreate_t)                    (RwRaster *raster);
typedef void                    (__cdecl *RwTextureUnlinkFromDictionary_t)      (RwTexture *texture);
typedef int                     (__cdecl *RwTextureDestroy_t)                   (RwTexture *texture);
typedef float                   (__cdecl *RwV3dNormalize_t)                     (RwV3d *out, const RwV3d *in);
typedef RwV3d*                  (__cdecl *RwV3dTransformVector_t)               (RwV3d *out, const RwV3d *in, const RwMatrix *matrix);
typedef RwRaster*               (__cdecl *RwRasterUnlock_t)                     (RwRaster *raster);
typedef RwRaster*               (__cdecl *RwRasterLock_t)                       (RwRaster *raster, unsigned char level, int lockmode);
typedef RwRaster*               (__cdecl *RwRasterCreate_t)                     (int width, int height, int depth, RwRasterType type);
typedef void                    (__cdecl *RwRasterDestroy_t)                    (RwRaster *raster);
typedef RwScene*                (__cdecl *RwSceneRender_t)                      (RwScene *scene);
typedef RwScene*                (__cdecl *RwSceneAddAtomic_t)                   (RwScene *scene, RpAtomic *atomic);
typedef RwScene*                (__cdecl *RwSceneAddClump_t)                    (RwScene *scene, RpClump *clump);
typedef RwScene*                (__cdecl *RwSceneAddLight_t)                    (RwScene *scene, RpLight *light);
typedef void                    (__cdecl *RwSceneDestroy_t)                     (RwScene *scene);
typedef void                    (__cdecl *RpPrtStdGlobalDataSetStreamEmbedded_t)(CBaseModelInfoSAInterface *info);
typedef RwError*                (__cdecl *RwErrorGet_t)                         (RwError *code);
typedef void                    (__cdecl *RwPrefetch_t)                         ();
typedef void                    (__cdecl *RwFlushLoader_t)                      ();

/*****************************************************************************/
/** Renderware function mappings                                            **/
/*****************************************************************************/

// US Versions
extern RwRenderSystemFigureAffairs_t            RwRenderSystemFigureAffairs;
extern RwErrorGet_t                             RwErrorGet;
extern RwAllocAligned_t                         RwAllocAligned;
extern RwFreeAligned_t                          RwFreeAligned;
extern RwCreateExtension_t                      RwCreateExtension;
extern RwObjectRegister_t                       RwObjectRegister;
extern RwStreamFindChunk_t                      RwStreamFindChunk;
extern RwStreamOpen_t                           RwStreamOpen;
extern RwStreamReadBlocks_t                     RwStreamReadBlocks;
extern RwStreamReadTexture_t                    RwStreamReadTexture;
extern RwStreamClose_t                          RwStreamClose;
extern RwFrameCreate_t                          RwFrameCreate;
extern RwFrameGetLTM_t                          RwFrameGetLTM;
extern RwFrameSetIdentity_t                     RwFrameSetIdentity;
extern RwFrameTranslate_t                       RwFrameTranslate;
extern RwFrameCloneHierarchy_t                  RwFrameCloneHierarchy;
extern RwFrameAddChild_t                        RwFrameAddChild;
extern RwFrameRemoveChild_t                     RwFrameRemoveChild;
extern RwFrameScale_t                           RwFrameScale;
extern RwFrameOrient_t                          RwFrameOrient;
extern RwFrameDestroy_t                         RwFrameDestroy;
extern RwCameraClone_t                          RwCameraClone;
extern RwCameraDestroy_t                        RwCameraDestroy;
extern RpClumpClone_t                           RpClumpClone;
extern RpClumpStreamRead_t                      RpClumpStreamRead;
extern RpClumpAddAtomic_t                       RpClumpAddAtomic;
extern RpClumpRemoveAtomic_t                    RpClumpRemoveAtomic;
extern RpClumpAddLight_t                        RpClumpAddLight;
extern RpClumpGetBoneTransform_t                RpClumpGetBoneTransform;
extern RpClumpSetupFrameCallback_t              RpClumpSetupFrameCallback;
extern RpClumpDestroy_t                         RpClumpDestroy;
extern RpClumpGetNumAtomics_t                   RpClumpGetNumAtomics;
extern RwAnimationInit_t                        RwAnimationInit;
extern RwSkeletonUpdate_t                       RwSkeletonUpdate;
extern RpAtomicCreate_t                         RpAtomicCreate;
extern RpAtomicClone_t                          RpAtomicClone;
extern RpAtomicSetFrame_t                       RpAtomicSetFrame;
extern RpAtomicSetupObjectPipeline_t            RpAtomicSetupObjectPipeline;
extern RpAtomicSetupVehiclePipeline_t           RpAtomicSetupVehiclePipeline;
extern RpAtomicRender_t                         RpAtomicRender;
extern RpAtomicRenderEx_t                       RpAtomicRenderEx;
extern RpAtomicSetGeometry_t                    RpAtomicSetGeometry;
extern RpAtomicDestroy_t                        RpAtomicDestroy;
extern RwObjectFrameRender_t                    RwObjectFrameRender;
extern RwTexDictionaryCreate_t                  RwTexDictionaryCreate;
extern RwTexDictionaryFinalizer_t               RwTexDictionaryFinalizer;
extern RwTexDictionaryStreamRead_t              RwTexDictionaryStreamRead;
extern RwTexDictionaryGetCurrent_t              RwTexDictionaryGetCurrent;
extern RwTexDictionarySetCurrent_t              RwTexDictionarySetCurrent;
extern RwTexDictionaryForAllTextures_t          RwTexDictionaryForAllTextures;
extern RwTexDictionaryFindNamedTexture_t        RwTexDictionaryFindNamedTexture;
extern RwTexDictionaryAddTexture_t              RwTexDictionaryAddTexture;
extern RwTexDictionaryDestroy_t                 RwTexDictionaryDestroy;
extern RwTextureCreate_t                        RwTextureCreate;
extern RwTextureUnlinkFromDictionary_t          RwTextureUnlinkFromDictionary;
extern RwTextureDestroy_t                       RwTextureDestroy;
extern RpGeometryCreate_t                       RpGeometryCreate;
extern RpGeometryAddRef_t                       RpGeomtryAddRef;
extern RpGeometryGetAnimation_t                 RpGeometryGetAnimation;
extern RpGeometryTriangleSetVertexIndices_t     RpGeometryTriangleSetVertexIndices;
extern RpGeometryTriangleSetMaterial_t          RpGeometryTriangleSetMaterial;
extern RpGeometryUnlock_t                       RpGeometryUnlock;
extern RpGeometryLock_t                         RpGeometryLock;
extern RpGeometryTransform_t                    RpGeometryTransform;
extern RpGeometryDestroy_t                      RpGeometryDestroy;
extern RwMatrixCreate_t                         RwMatrixCreate;
extern RwMatrixInvert_t                         RwMatrixInvert;
extern RwMatrixTranslate_t                      RwMatrixTranslate;
extern RwMatrixScale_t                          RwMatrixScale;
extern RwMatrixUnknown_t                        RwMatrixUnknown;
extern RpMaterialCreate_t                       RpMaterialCreate;
extern RpMaterialDestroy_t                      RpMaterialDestroy;
extern RwV3dNormalize_t                         RwV3dNormalize;
extern RwV3dTransformVector_t                   RwV3dTransformVector;
extern RwIm3DTransform_t                        RwIm3DTransform;
extern RwIm3DRenderIndexedPrimitive_t           RwIm3DRenderIndexedPrimitive;
extern RwIm3DEnd_t                              RwIm3DEnd;
extern RpLightSetRadius_t                       RpLightSetRadius;
extern RpLightDestroy_t                         RpLightDestroy;
extern RwRasterCreate_t                         RwRasterCreate;
extern RwRasterUnlock_t                         RwRasterUnlock;
extern RwRasterLock_t                           RwRasterLock;
extern RwRasterDestroy_t                        RwRasterDestroy;
extern RwSceneAddAtomic_t                       RwSceneAddAtomic;
extern RwSceneAddLight_t                        RwSceneAddLight;
extern RwSceneAddClump_t                        RwSceneAddClump;
extern RpPrtStdGlobalDataSetStreamEmbedded_t    RpPrtStdGlobalDataSetStreamEmbedded;
extern RwPrefetch_t                             RwPrefetch;
extern RwFlushLoader_t                          RwFlushLoader;

/*****************************************************************************/
/** GTA function definitions                                                **/
/*****************************************************************************/

typedef bool                (__cdecl *SetTextureDict_t)                 (unsigned short id);
typedef bool                (__cdecl *LoadClumpFile_t)                  (RwStream *stream, unsigned int id);        // (stream, model id)
typedef bool                (__cdecl *LoadModel_t)                      (RwBuffer *filename, unsigned int id);      // (memory chunk, model id)
typedef void                (__cdecl *LoadCollisionModel_t)             (unsigned char*, CColModelSAInterface*, const char*);
typedef void                (__cdecl *LoadCollisionModelVer2_t)         (unsigned char*, unsigned int, CColModelSAInterface*, const char*);
typedef void                (__cdecl *LoadCollisionModelVer3_t)         (unsigned char*, unsigned int, CColModelSAInterface*, const char*); // buf, bufsize, ccolmodel&, keyname
typedef bool                (__cdecl *CTxdStore_LoadTxd_t)              (unsigned int id, RwStream *filename);
typedef void                (__cdecl *CTxdStore_RemoveTxd_t)            (unsigned int id);
typedef void                (__cdecl *CTxdStore_RemoveRef_t)            (unsigned int id);
typedef void                (__cdecl *CTxdStore_AddRef_t)               (unsigned int id);
typedef RwTexDictionary*    (__cdecl *CTxdStore_GetTxd_t)               (unsigned int id);
typedef RwTexture*          (__cdecl *CClothesBuilder_CopyTexture_t)    (RwTexture *texture);

extern SetTextureDict_t                 SetTextureDict;
extern LoadClumpFile_t                  LoadClumpFile;
extern LoadModel_t                      LoadModel;
extern LoadCollisionModel_t             LoadCollisionModel;
extern LoadCollisionModelVer2_t         LoadCollisionModelVer2;
extern LoadCollisionModelVer3_t         LoadCollisionModelVer3;
extern CTxdStore_LoadTxd_t              CTxdStore_LoadTxd;
extern CTxdStore_GetTxd_t               CTxdStore_GetTxd;
extern CTxdStore_RemoveTxd_t            CTxdStore_RemoveTxd;
extern CTxdStore_RemoveRef_t            CTxdStore_RemoveRef;
extern CTxdStore_AddRef_t               CTxdStore_AddRef;
extern CClothesBuilder_CopyTexture_t    CClothesBuilder_CopyTexture;

/*****************************************************************************/
/** Function inlines                                                        **/
/*****************************************************************************/

#endif //__GAMESA_RENDERWARE