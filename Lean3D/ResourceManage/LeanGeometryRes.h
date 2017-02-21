#pragma  once

#include "LeanUtil.h"
#include "utMath.h"
#include "LeanResource.h"

namespace Lean3D
{
	struct GeometryElemType
	{
		enum List
		{
			GeometryElem = 1000,
			GeoIndexCountI,
			GeoVertexCountI,
			GeoIndices16I,
			GeoIndexStream,
			GeoVertPosStream,
			GeoVertTanSteam,
			GeoVertStaticStream
		};
	};

	struct VertexTanData
	{
		Vec3  normal;
		Vec3  tangent;
		float handedness;
	};

	struct VertexStaticData
	{
		float  u0, v0;
		float  jointVec[4];//每个顶点最多受4个关节影响
		float  weightVec[4];//每个关节影响的权重
		float  u1, v1;
	};

	struct Joint
	{
		Matrix4  invBindMat;
	};

	struct MorphDiff
	{
		uint32  vertIndex;
		Vec3	posDiff;
		Vec3	normDiff;
		Vec3	tanDiff;
	};

	struct MorphTarget
	{
		std::string               name;
		std::vector< MorphDiff >  diffs;
	};

	class GeometryResource : public Resource
	{
	public:
		GeometryResource(const std::string &name, int flags);
		~GeometryResource();
		Resource *clone();

		static void initFunc();
		static void releaseFunc();
		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return new GeometryResource(name, flags);
		}

		void initDefault();
		void release();
		bool load(const char *data, int size);

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);
		//void *mapStream(int elem, int elemIdx, int stream, bool read, bool write);
		//void unmapStream();

		uint32 getVAOHandle(){ return _geomVAOHandle; }
		VertexAttribLayoutList &getAttribList(){ return _vertAttribList; }
		uint32 getVertCount() { return _vertCount; }
		uint32 getIndexCount() { return _indexCount; }
		uint32 getPosVBOHandle() { return _posVBOHandle; }
		uint32 getTanVBOHandle() { return _tanVBOHandle; }
		uint32 getStaticVBOHandle() { return _staticVBOHandle; }
		uint32 getIndexVBOHandle() { return _indexVBOHandle; }
		char *getIndexData(){ return _indexData; }
		VertexTanData *getVertTanData() { return _vertTanData; }
		VertexStaticData *getVertStaticData() { return _vertStaticData; }
		
		void updateDynaDataToGpu();
		
		void genGeomResVAO(uint32 shaderHandle);

		static uint32 defVertBuffer, defIndexBuffer; //所有对象共用默认VBO，IBO索引
	protected:
		
	private:
		static int                  mappedWriteStream;

		uint32                      _indexVBOHandle;
		uint32						_posVBOHandle;
		uint32						_tanVBOHandle;
		uint32						_staticVBOHandle;
		uint32						_geomVAOHandle;

		VertexAttribLayoutList		_vertAttribList;
		uint32                      _indexCount, _vertCount;
		bool                        _16BitIndices;
		char                        *_indexData;
		Vec3                        *_vertPosData;
		VertexTanData               *_vertTanData;
		VertexStaticData            *_vertStaticData;

		std::vector< Joint >        _joints;    //关节空间逆变矩阵
		BoundingBox                 _skelAABB;	//骨骼AABB
		std::vector< MorphTarget >  _morphTargets;
		uint32                      _minMorphIndex, _maxMorphIndex;
	};

}