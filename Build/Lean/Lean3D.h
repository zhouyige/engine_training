#pragma once

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#  if defined( __GNUC__ ) && __GNUC__ >= 4
#   define DLL extern "C" __attribute__ ((visibility("default")))
#  else
#		define DLL extern "C"
#  endif
#	endif
#endif



typedef int ResHandle;
typedef int NodeHandle;

const NodeHandle L3DRootNode = 1;


struct L3DOptions
{
	enum List
	{
		MaxLogLevel = 1,
		MaxNumMessages,
		TrilinearFiltering,
		MaxAnisotropy,
		TexCompression,
		SRGBLinearization,
		LoadTextures,
		FastAnimation,
		ShadowMapSize,
		SampleCount,
		WireframeMode,
		DebugViewMode,
		DumpFailedShaders,
		GatherTimeStats
	};
};

struct L3DStats
{
	enum List
	{
		TriCount = 100,
		BatchCount,
		LightPassCount,
		FrameTime,
		AnimationTime,
		GeoUpdateTime,
		ParticleSimTime,
		FwdLightsGPUTime,
		DefLightsGPUTime,
		ShadowsGPUTime,
		ParticleGPUTime,
		TextureVMem,
		GeometryVMem
	};
};

struct L3DResTypes
{
	enum List
	{
		Undefined = 0,
		SceneGraph,
		Geometry,
		Animation,
		Material,
		Code,
		Shader,
		Texture,
		ParticleEffect,
		Pipeline
	};
};

struct L3DResFlags
{
	enum Flags
	{
		NoQuery = 1,
		NoTexCompression = 2,
		NoTexMipmaps = 4,
		TexCubemap = 8,
		TexDynamic = 16,
		TexRenderable = 32,
		TexSRGB = 64
	};
};


struct L3DFormats
{
	enum List
	{
		Unknown = 0,
		TEX_BGRA8,
		TEX_DXT1,
		TEX_DXT3,
		TEX_DXT5,
		TEX_RGBA16F,
		TEX_RGBA32F
	};
};


struct L3DGeoRes
{
	enum List
	{
		GeometryElem = 200,
		GeoIndexCountI,
		GeoVertexCountI,
		GeoIndices16I,
		GeoIndexStream,
		GeoVertPosStream,
		GeoVertTanStream,
		GeoVertStaticStream
	};
};

struct L3DAnimRes
{
	enum List
	{
		EntityElem = 300,
		EntFrameCountI
	};
};

struct L3DMatRes
{
	enum List
	{
		MaterialElem = 400,
		SamplerElem,
		UniformElem,
		MatClassStr,
		MatLinkI,
		MatShaderI,
		SampNameStr,
		SampTexResI,
		UnifNameStr,
		UnifValueF4
	};
};

struct L3DShaderRes
{
	enum List
	{
		ContextElem = 600,
		SamplerElem,
		UniformElem,
		ContNameStr,
		SampNameStr,
		SampDefTexResI,
		UnifNameStr,
		UnifSizeI,
		UnifDefValueF4
	};
};

struct L3DTexRes
{
	enum List
	{
		TextureElem = 700,
		ImageElem,
		TexFormatI,
		TexSliceCountI,
		ImgWidthI,
		ImgHeightI,
		ImgPixelStream
	};
};

struct L3DPartEffRes
{
	enum List
	{
		ParticleElem = 800,
		ChanMoveVelElem,
		ChanRotVelElem,
		ChanSizeElem,
		ChanColRElem,
		ChanColGElem,
		ChanColBElem,
		ChanColAElem,
		PartLifeMinF,
		PartLifeMaxF,
		ChanStartMinF,
		ChanStartMaxF,
		ChanEndRateF,
		ChanDragElem
	};
};

struct L3DPipeRes
{
	enum List
	{
		StageElem = 900,
		StageNameStr,
		StageActivationI
	};
};


struct L3DNodeTypes
{
	enum List
	{
		Undefined = 0,
		Group,
		Model,
		Mesh,
		Joint,
		Light,
		Camera,
		Emitter
	};
};

struct L3DNodeFlags
{
	enum List
	{
		NoDraw = 1,
		NoCastShadow = 2,
		NoRayQuery = 4,
		Inactive = 7  // NoDraw | NoCastShadow | NoRayQuery
	};
};

struct L3DNodeParams
{
	enum List
	{
		NameStr = 1,
		AttachmentStr
	};
};


struct L3DModel
{
	enum List
	{
		GeoResI = 200,
		SWSkinningI,
		LodDist1F,
		LodDist2F,
		LodDist3F,
		LodDist4F
	};
};

struct L3DMesh
{
	enum List
	{
		MatResI = 300,
		BatchStartI,
		BatchCountI,
		VertRStartI,
		VertREndI,
		LodLevelI
	};
};

struct L3DJoint
{
	enum List
	{
		JointIndexI = 400
	};
};

struct L3DLight
{
	enum List
	{
		MatResI = 500,
		RadiusF,
		FovF,
		ColorF3,
		ColorMultiplierF,
		ShadowMapCountI,
		ShadowSplitLambdaF,
		ShadowMapBiasF,
		LightingContextStr,
		ShadowContextStr
	};
};

struct L3DCamera
{
	enum List
	{
		PipeResI = 600,
		OutTexResI,
		OutBufIndexI,
		LeftPlaneF,
		RightPlaneF,
		BottomPlaneF,
		TopPlaneF,
		NearPlaneF,
		FarPlaneF,
		ViewportXI,
		ViewportYI,
		ViewportWidthI,
		ViewportHeightI,
		OrthoI,
		OccCullingI
	};
};

struct L3DEmitter
{
	enum List
	{
		MatResI = 700,
		PartEffResI,
		MaxCountI,
		RespawnCountI,
		DelayF,
		EmissionRateF,
		SpreadAngleF,
		ForceF3
	};
};


struct L3DModelUpdateFlags
{
	enum Flags
	{
		Animation = 1,
		Geometry = 2
	};
};


DLL const char *LeanGetVersionString();

DLL bool LeanCheckExtension( const char *extensionName );

DLL bool LeanGetError();

DLL bool LeanInit();

DLL void LeanRelease();


DLL void LeanRender( NodeHandle cameraNode );

DLL void LeanFinalizeFrame();

DLL void LeanClear();


DLL const char *LeanGetMessage( int *level, float *time );

DLL float LeanGetOption( L3DOptions::List param );

DLL bool LeanSetOption( L3DOptions::List param, float value );

DLL float LeanGetStat( L3DStats::List param, bool reset );

DLL void LeanShowOverlays( const float *verts, int vertCount, float colR, float colG, float colB,
                          float colA, ResHandle materialRes, int flags );

DLL void LeanClearOverlays();


DLL int LeanGetResType( ResHandle res );

DLL const char *LeanGetResName( ResHandle res );

DLL ResHandle LeanGetNextResource( int type, ResHandle start );

DLL ResHandle LeanFindResource( int type, const char *name );

DLL ResHandle LeanAddResource( int type, const char *name, int flags );

DLL ResHandle LeanCloneResource( ResHandle sourceRes, const char *name );

DLL int LeanRemoveResource( ResHandle res );

DLL bool LeanIsResLoaded( ResHandle res );

DLL bool LeanLoadResource( ResHandle res, const char *data, int size );

DLL void LeanUnloadResource( ResHandle res );


DLL int LeanGetResElemCount( ResHandle res, int elem );

DLL int LeanFindResElem( ResHandle res, int elem, int param, const char *value );

DLL int LeanGetResParamI( ResHandle res, int elem, int elemIdx, int param );

DLL void LeanSetResParamI( ResHandle res, int elem, int elemIdx, int param, int value );

DLL float LeanGetResParamF( ResHandle res, int elem, int elemIdx, int param, int compIdx );

DLL void LeanSetResParamF( ResHandle res, int elem, int elemIdx, int param, int compIdx, float value );

DLL const char *LeanGetResParamStr( ResHandle res, int elem, int elemIdx, int param );

DLL void LeanSetResParamStr( ResHandle res, int elem, int elemIdx, int param, const char *value );

DLL void *LeanMapResStream( ResHandle res, int elem, int elemIdx, int stream, bool read, bool write );

DLL void LeanUnmapResStream( ResHandle res );

DLL ResHandle LeanQueryUnloadedResource( int index );

DLL void LeanReleaseUnusedResources();


DLL ResHandle LeanCreateTexture( const char *name, int width, int height, int fmt, int flags );

DLL void LeanSetShaderPreambles( const char *vertPreamble, const char *fragPreamble );

DLL bool LeanSetMaterialUniform( ResHandle materialRes, const char *name, float a, float b, float c, float d );

DLL void LeanResizePipelineBuffers( ResHandle pipeRes, int width, int height );

DLL bool LeanGetRenderTargetData( ResHandle pipelineRes, const char *targetName, int bufIndex,
                                 int *width, int *height, int *compCount, void *dataBuffer, int bufferSize );


DLL int LeanGetNodeType( NodeHandle node );
	
DLL NodeHandle LeanGetNodeParent( NodeHandle node );


DLL bool LeanSetNodeParent( NodeHandle node, NodeHandle parent );

DLL NodeHandle LeanGetNodeChild( NodeHandle node, int index );



DLL NodeHandle LeanAddNodes( NodeHandle parent, ResHandle sceneGraphRes );

DLL void LeanRemoveNode( NodeHandle node );

DLL bool LeanCheckNodeTransFlag( NodeHandle node, bool reset );

DLL void LeanGetNodeTransform( NodeHandle node, float *tx, float *ty, float *tz,
                              float *rx, float *ry, float *rz, float *sx, float *sy, float *sz );

DLL void LeanSetNodeTransform( NodeHandle node, float tx, float ty, float tz,
                              float rx, float ry, float rz, float sx, float sy, float sz );

DLL void LeanGetNodeTransMats( NodeHandle node, const float **relMat, const float **absMat );

DLL void LeanSetNodeTransMat( NodeHandle node, const float *mat4x4 );

DLL int LeanGetNodeParamI( NodeHandle node, int param );

DLL void LeanSetNodeParamI( NodeHandle node, int param, int value );

DLL float LeanGetNodeParamF( NodeHandle node, int param, int compIdx );

DLL void LeanSetNodeParamF( NodeHandle node, int param, int compIdx, float value );

DLL const char *LeanGetNodeParamStr( NodeHandle node, int param );

DLL void LeanSetNodeParamStr( NodeHandle node, int param, const char *value );

DLL int LeanGetNodeFlags( NodeHandle node );

DLL void LeanSetNodeFlags( NodeHandle node, int flags, bool recursive );

DLL void LeanGetNodeAABB( NodeHandle node, float *minX, float *minY, float *minZ,
                         float *maxX, float *maxY, float *maxZ );

DLL int LeanFindNodes( NodeHandle startNode, const char *name, int type );

DLL NodeHandle LeanGetNodeFindResult( int index );

DLL void LeanSetNodeUniforms( NodeHandle node, float *uniformData, int count );

DLL int LeanCastRay( NodeHandle node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest );


DLL bool LeanGetCastRayResult( int index, NodeHandle *node, float *distance, float *intersection );

DLL int LeanCheckNodeVisibility( NodeHandle node, NodeHandle cameraNode, bool checkOcclusion, bool calcLod );


DLL NodeHandle LeanAddGroupNode( NodeHandle parent, const char *name );


DLL NodeHandle LeanAddModelNode( NodeHandle parent, const char *name, ResHandle geometryRes );

DLL void LeanSetupModelAnimStage( NodeHandle modelNode, int stage, ResHandle animationRes, int layer,
                                 const char *startNode, bool additive );

DLL void LeanSetModelAnimParams( NodeHandle modelNode, int stage, float time, float weight );

DLL bool LeanSetModelMorpher( NodeHandle modelNode, const char *target, float weight );


DLL void LeanUpdateModel( NodeHandle modelNode, int flags );


DLL NodeHandle LeanAddMeshNode( NodeHandle parent, const char *name, ResHandle materialRes, 
                            int batchStart, int batchCount, int vertRStart, int vertREnd );


DLL NodeHandle LeanAddJointNode( NodeHandle parent, const char *name, int jointIndex );


DLL NodeHandle LeanAddLightNode( NodeHandle parent, const char *name, ResHandle materialRes,
                             const char *lightingContext, const char *shadowContext );


DLL NodeHandle LeanAddCameraNode( NodeHandle parent, const char *name, ResHandle pipelineRes );

DLL void LeanSetupCameraView( NodeHandle cameraNode, float fov, float aspect, float nearDist, float farDist );

DLL void LeanGetCameraProjMat( NodeHandle cameraNode, float *projMat );


DLL NodeHandle LeanAddEmitterNode( NodeHandle parent, const char *name, ResHandle materialRes,
                               ResHandle particleEffectRes, int maxParticleCount, int respawnCount );

DLL void LeanUpdateEmitter( NodeHandle emitterNode, float timeDelta );

DLL bool LeanHasEmitterFinished( NodeHandle emitterNode );
