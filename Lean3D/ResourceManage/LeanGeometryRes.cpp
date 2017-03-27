#include "LeanGeometryRes.h"
#include "OGLDeviceManager.h"
#include "Lean3DRoot.h"
#include <algorithm>

namespace Lean3D
{

	uint32 GeometryResource::defVertBuffer = 0;
	uint32 GeometryResource::defIndexBuffer = 0;

	GeometryResource::GeometryResource(const std::string &name, int flags):
		Resource(ResourceTypes::Geometry, name, flags)
	{
		initDefault();
	}

	GeometryResource::~GeometryResource()
	{
		release();
	}

	void GeometryResource::initDefault()
	{
		_geomVAOHandle = 0;
		_indexCount = 0;
		_vertCount = 0;
		_indexData = 0x0;
		_vertPosData = 0x0;
		_vertTanData = 0x0;
		_vertStaticData = 0x0;
		_16BitIndices = false;
		_indexVBOHandle = defIndexBuffer;
		_posVBOHandle = defVertBuffer;
		_tanVBOHandle = defVertBuffer;
		_staticVBOHandle = defVertBuffer;
		_minMorphIndex = 0; _maxMorphIndex = 0;
		_skelAABB.min = Vec3(0, 0, 0);
		_skelAABB.max = Vec3(0, 0, 0);
	}

	void GeometryResource::release()
	{
		if (_posVBOHandle != 0 && _posVBOHandle != defVertBuffer)
		{
			g_OGLDiv->buffManaRef()->destroyBuffer(_posVBOHandle);
			_posVBOHandle = 0;
		}
		if (_tanVBOHandle != 0 && _tanVBOHandle != defVertBuffer)
		{
			g_OGLDiv->buffManaRef()->destroyBuffer(_tanVBOHandle);
			_tanVBOHandle = 0;
		}
		if (_staticVBOHandle != 0 && _staticVBOHandle != defVertBuffer)
		{
			g_OGLDiv->buffManaRef()->destroyBuffer(_staticVBOHandle);
			_staticVBOHandle = 0;
		}

		if (_indexVBOHandle != 0 && _indexVBOHandle != defIndexBuffer)
		{
			g_OGLDiv->buffManaRef()->destroyBuffer(_indexVBOHandle);
			_indexVBOHandle = 0;
		}

		delete[] _indexData; _indexData = 0x0;
		delete[] _vertPosData; _vertPosData = 0x0;
		delete[] _vertTanData; _vertTanData = 0x0;
		delete[] _vertStaticData; _vertStaticData = 0x0;
		_joints.clear();
		_morphTargets.clear();
	}

	Resource * GeometryResource::clone()
	{
		GeometryResource *res = new GeometryResource("", _flags);

		*res = *this;

		res->_indexData = new char[_indexCount * (_16BitIndices ? 2 : 4)];
		res->_vertPosData = new Vec3[_vertCount];
		res->_vertTanData = new VertexTanData[_vertCount];
		res->_vertStaticData = new VertexStaticData[_vertCount];
		memcpy(res->_indexData, _indexData, _indexCount * (_16BitIndices ? 2 : 4));
		memcpy(res->_vertPosData, _vertPosData, _vertCount * sizeof(Vec3));
		memcpy(res->_vertTanData, _vertTanData, _vertCount * sizeof(VertexTanData));
		memcpy(res->_vertStaticData, _vertStaticData, _vertCount * sizeof(VertexStaticData));
		res->_indexVBOHandle = g_OGLDiv->buffManaRef()->createIndexBuffer(_indexCount * (_16BitIndices ? 2 : 4), _indexData);
		res->_posVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount * sizeof(Vec3), _vertPosData);
		res->_tanVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount * sizeof(VertexTanData), _vertTanData);
		res->_staticVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount * sizeof(VertexStaticData), _vertStaticData);
	
		return res;
	}

	void GeometryResource::initFunc()
	{
		defVertBuffer = g_OGLDiv->buffManaRef()->createVertexBuffer(0, 0x0);
		defIndexBuffer = g_OGLDiv->buffManaRef()->createIndexBuffer(0, 0x0);
	}

	void GeometryResource::releaseFunc()
	{
		g_OGLDiv->buffManaRef()->destroyBuffer(defVertBuffer);
		g_OGLDiv->buffManaRef()->destroyBuffer(defIndexBuffer);
	}

	bool GeometryResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size)) return false;
		
		if (size < 8)
		{
			LEAN_DEGUG_LOG("Error ��Ч�����ļ���", 0);
			return false;
		}

		char *pData = (char*)data;

		//����ļ�ͷ�Ͱ汾��
		char header[4];
		memcpy(&header, pData, 4); 
		pData += 4;
		if (header[0] != 'H' || header[1] != '3' || header[2] != 'D' || header[3] != 'G')
		{
			LEAN_DEGUG_LOG("Error ��Ч�����ļ���", 0);
			return false;
		}
		uint32 version;
		memcpy(&version, pData, sizeof(uint32));
		pData += sizeof(uint32);
		if (version != 1 && version != 5)
		{
			LEAN_DEGUG_LOG("Error ��Ч�����ļ���", 0);
			return false;
		}

		//��ȡ�ؽ���任����
		uint32 count;
		memcpy(&count, pData, sizeof(uint32));
		pData += sizeof(uint32);
		if (count > 75)
			LEAN_DEGUG_LOG("Warning Joints����75����", 0);

		_joints.resize(count);
		for (uint32 i = 0; i < count; ++i)
		{
			Joint &joint = _joints[i];
			for (uint32 j = 0; j < 16; ++j)
			{
				memcpy(&joint.invBindMat.x[j], pData, sizeof(float));
				pData += sizeof(float);
			}
		}

		//��ȡ����������
		uint32 streamSize;
		memcpy(&count, pData, sizeof(uint32));
		pData += sizeof(uint32);
		memcpy(&streamSize, pData, sizeof(uint32));
		pData += sizeof(uint32);

		_vertCount = streamSize;
		_vertPosData = new Vec3[_vertCount];
		_vertTanData = new VertexTanData[_vertCount];
		_vertStaticData = new VertexStaticData[_vertCount];
		Vec3 *bitangents = new Vec3[_vertCount];
		//���ݳ�ʼ��
		memset(_vertPosData, 0, _vertCount * sizeof(Vec3));
		memset(_vertTanData, 0, _vertCount * sizeof(VertexTanData));
		memset(_vertStaticData, 0, _vertCount * sizeof(VertexStaticData));
		for (uint32 i = 0; i < _vertCount; ++i)
			_vertStaticData[i].weightVec[0] = 1;
		for (uint32 i = 0; i < count; ++i)
		{
			unsigned char uchar;
			short sht;
			uint32 streamID, streamElemSize;
			//ÿ������Ԫ������
			memcpy(&streamID, pData, sizeof(uint32));
			pData += sizeof(uint32);
			//ÿ������Ԫ��ռ�ÿռ�
			memcpy(&streamElemSize, pData, sizeof(uint32));
			pData += sizeof(uint32);

			switch (streamID)
			{
			case 0:		//����Pos
				if (streamElemSize != 12)
				{
					LEAN_DEGUG_LOG("Error ����pos����������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&_vertPosData[j].x, pData, sizeof(float));
					pData += sizeof(float);
					memcpy(&_vertPosData[j].y, pData, sizeof(float));
					pData += sizeof(float);
					memcpy(&_vertPosData[j].z, pData, sizeof(float));
					pData += sizeof(float);
				}
				break;
			case 1:		//����
				if (streamElemSize != 6)
				{
					LEAN_DEGUG_LOG("Error ����normal����������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short);
					_vertTanData[j].normal.x = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short);
					_vertTanData[j].normal.y = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short));
					pData += sizeof(short); 
					_vertTanData[j].normal.z = sht / 32767.0f;
				}
				break;
			case 2:		//����
				if (streamElemSize != 6)
				{
					LEAN_DEGUG_LOG("Error ������������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short); 
					_vertTanData[j].tangent.x = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short); 
					_vertTanData[j].tangent.y = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short);
					_vertTanData[j].tangent.z = sht / 32767.0f;
				}
				break;
			case 3:		//�����ߣ�˫���ߣ�
				if (streamElemSize != 6)
				{
					LEAN_DEGUG_LOG("Error ���㸱��������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&sht, pData, sizeof(short));
					pData += sizeof(short); 
					bitangents[j].x = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short); 
					bitangents[j].y = sht / 32767.0f;
					memcpy(&sht, pData, sizeof(short)); 
					pData += sizeof(short); 
					bitangents[j].z = sht / 32767.0f;
				}
				break;
			case 4:		//��������Ĺؽ�����
				if (streamElemSize != 4)
				{
					LEAN_DEGUG_LOG("Error ����ؽ���������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&uchar, pData, sizeof(char)); 
					pData += sizeof(char); 
					_vertStaticData[j].jointVec[0] = (float)uchar;
					memcpy(&uchar, pData, sizeof(char));
					pData += sizeof(char); 
					_vertStaticData[j].jointVec[1] = (float)uchar;
					memcpy(&uchar, pData, sizeof(char));
					pData += sizeof(char); 
					_vertStaticData[j].jointVec[2] = (float)uchar;
					memcpy(&uchar, pData, sizeof(char));
					pData += sizeof(char); 
					_vertStaticData[j].jointVec[3] = (float)uchar;
				}
				break;
			case 5:		//�����ܹؽ�Ӱ���Ȩ��
				if (streamElemSize != 4)
				{
					LEAN_DEGUG_LOG("Error Ȩ������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&uchar, pData, sizeof(char)); 
					pData += sizeof(char); 
					_vertStaticData[j].weightVec[0] = uchar / 255.0f;
					memcpy(&uchar, pData, sizeof(char)); 
					pData += sizeof(char); 
					_vertStaticData[j].weightVec[1] = uchar / 255.0f;
					memcpy(&uchar, pData, sizeof(char)); 
					pData += sizeof(char); 
					_vertStaticData[j].weightVec[2] = uchar / 255.0f;
					memcpy(&uchar, pData, sizeof(char)); 
					pData += sizeof(char); 
					_vertStaticData[j].weightVec[3] = uchar / 255.0f;
				}
				break;
			case 6:		//��������
				if (streamElemSize != 8)
				{
					LEAN_DEGUG_LOG("Error ����1��������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&_vertStaticData[j].u0, pData, sizeof(float)); 
					pData += sizeof(float);
					memcpy(&_vertStaticData[j].v0, pData, sizeof(float)); 
					pData += sizeof(float);
				}
				break;
			case 7:		//���õڶ�����������
				if (streamElemSize != 8)
				{
					LEAN_DEGUG_LOG("Error ����2��������������", 0);
					break;
				}
				for (uint32 j = 0; j < streamSize; ++j)
				{
					memcpy(&_vertStaticData[j].u0, pData, sizeof(float));
					pData += sizeof(float);
					memcpy(&_vertStaticData[j].v0, pData, sizeof(float));
					pData += sizeof(float);
				}
				break;
			default:
				pData += streamSize * streamSize;
				LEAN_DEGUG_LOG("Warning δ֪���������������", 0);
				continue;
			}
		}
		
		//�ж�geo�ļ��еĸ����ߺͼ���ĸ����߼н����� ��ֵΪ��˵������ͼ����ļ�ʹ������ϵ��ͬ
		for (uint32 i = 0; i < _vertCount; ++i)
		{
			_vertTanData[i].handedness = _vertTanData[i].normal.cross(_vertTanData[i].tangent).dot(bitangents[i]) < 0 ? -1.0f : 1.0f;
		}
		delete[] bitangents; bitangents = 0x0;

		//��ȡ����������
		memcpy(&count, pData, sizeof(uint32));
		pData += sizeof(uint32);
		_indexCount = count;
		_16BitIndices = _vertCount <= 65536;
		_indexData = new char[count * (_16BitIndices ? 2 : 4)];
		if (_16BitIndices)
		{
			uint32 index;
			uint16 *pIndexData = (uint16 *)_indexData;
			for (uint32 i = 0; i < count; ++i)
			{
				memcpy(&index, pData, sizeof(uint32)); pData += sizeof(uint32);
				pIndexData[i] = (uint16)index;
			}
		}
		else
		{
			uint32 *pIndexData = (uint32 *)_indexData;
			for (uint32 i = 0; i < count; ++i)
			{
				memcpy(&pIndexData[i], pData, sizeof(uint32)); pData += sizeof(uint32);
			}
		}

		//��ȡ����Ŀ�궯���������
		uint32 tagCount;
		memcpy(&tagCount, pData, sizeof(uint32));
		pData += sizeof(uint32);

		_morphTargets.resize(tagCount);
		for (uint32 i = 0; i < tagCount; ++i)
		{
			MorphTarget &mt = _morphTargets[i];
			char namestr[256];
			memcpy(namestr, pData, 256);
			pData += 256;
			mt.name = namestr;

			//��ȡ��������
			uint32 morphStreamSize;
			memcpy(&morphStreamSize, pData, sizeof(uint32));
			pData += sizeof(uint32);
			mt.diffs.resize(morphStreamSize);
			for (uint32 j = 0; j < morphStreamSize; ++j)
			{
				memcpy(&mt.diffs[j].vertIndex, pData, sizeof(uint32));
				pData += sizeof(uint32);
			}
			//��ȡ�������������
			memcpy(&count, pData, sizeof(uint32));
			pData += sizeof(uint32);
			for (uint32 j = 0; j < count; ++j)
			{
				uint32 streamID, streamElemSize;
				memcpy(&streamID, pData, sizeof(uint32)); 
				pData += sizeof(uint32);
				memcpy(&streamElemSize, pData, sizeof(uint32));
				pData += sizeof(uint32);

				switch (streamID)
				{
				case 0:		// ����pos
					if (streamElemSize != 12)
					{
						LEAN_DEGUG_LOG("Error ���ζ���pos����������", 0);
						break;
					}
					for (uint32 k = 0; k < morphStreamSize; ++k)
					{
						memcpy(&mt.diffs[k].posDiff.x, pData, sizeof(float));
						pData += sizeof(float);
						memcpy(&mt.diffs[k].posDiff.y, pData, sizeof(float));
						pData += sizeof(float);
						memcpy(&mt.diffs[k].posDiff.z, pData, sizeof(float));
						pData += sizeof(float);
					}
					break;
				case 1:		// ����
					if (streamElemSize != 12)
					{
						LEAN_DEGUG_LOG("Error ���η�������������", 0);
						break;
					}
					for (uint32 k = 0; k < morphStreamSize; ++k)
					{
						memcpy(&mt.diffs[k].normDiff.x, pData, sizeof(float));
						pData += sizeof(float);
						memcpy(&mt.diffs[k].normDiff.y, pData, sizeof(float));
						pData += sizeof(float);
						memcpy(&mt.diffs[k].normDiff.z, pData, sizeof(float));
						pData += sizeof(float);
					}
					break;
				case 2:		// ����
					if (streamElemSize != 12)
					{
						LEAN_DEGUG_LOG("Error ������������������", 0);
						break;
					}
					for (uint32 k = 0; k < morphStreamSize; ++k)
					{
						memcpy(&mt.diffs[k].tanDiff.x, pData, sizeof(float)); 
						pData += sizeof(float);
						memcpy(&mt.diffs[k].tanDiff.y, pData, sizeof(float));
						pData += sizeof(float);
						memcpy(&mt.diffs[k].tanDiff.z, pData, sizeof(float));
						pData += sizeof(float);
					}
					break;
				case 3:		// ������
					if (streamElemSize != 12)
					{
						LEAN_DEGUG_LOG("Error ���θ���������������", 0);
						break;
					}

					// ����ĸ�ʽ�б���Ŀ�겻�洢�����ߣ�����ֱ������
					pData += morphStreamSize * sizeof(float)* 3;
					break;
				default:
					pData += streamElemSize * morphStreamSize;
					LEAN_DEGUG_LOG("Error �޴˼��α�������Ԫ�أ�", 0);
					continue;
				}
			}
		}

		//������ѯ ����Ŀ���������С����  ȷ����С�������ݷ��ʷ�Χ
		_minMorphIndex = (unsigned int)_vertCount;
		_maxMorphIndex = 0;
		for (uint32 i = 0; i < _morphTargets.size(); ++i)
		{
			for (uint32 j = 0; j < _morphTargets[i].diffs.size(); ++j)
			{
				_minMorphIndex = std::min(_minMorphIndex, _morphTargets[i].diffs[j].vertIndex);
				_maxMorphIndex = std::max(_maxMorphIndex, _morphTargets[i].diffs[j].vertIndex);
			}
		}
		if (_minMorphIndex > _maxMorphIndex)
		{
			_minMorphIndex = _maxMorphIndex = 0;
		}
		//��ѯ�������¹�����AABB��
		for (uint32 i = 0; i < (uint32)_joints.size(); ++i)
		{
			//ģ������ϵԭ��任���ڵ�ԭ��
			Vec3 pos = _joints[i].invBindMat.inverted() * Vec3(0, 0, 0);
			if (pos.x < _skelAABB.min.x) _skelAABB.min.x = pos.x;
			if (pos.y < _skelAABB.min.y) _skelAABB.min.y = pos.y;
			if (pos.z < _skelAABB.min.z) _skelAABB.min.z = pos.z;
			if (pos.x > _skelAABB.max.x) _skelAABB.max.x = pos.x;
			if (pos.y > _skelAABB.max.y) _skelAABB.max.y = pos.y;
			if (pos.z > _skelAABB.max.z) _skelAABB.max.z = pos.z;
		}
		//����ؽ�Ϊ�� ���һ��Ĭ�Ϲؽڱ�ʾ����ģ��ֻ��һ���ؽ�
		if (_joints.empty())
		{
			_joints.push_back(Joint());
		}
		//�ϴ����ݵ�GPU
		if (_vertCount > 0 && _indexCount > 0)
		{
			//�����������
			_indexVBOHandle = g_OGLDiv->buffManaRef()->createIndexBuffer(_indexCount * (_16BitIndices ? 2 : 4), _indexData);
			//��Ӷ��㻺��
			_posVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount*sizeof(Vec3), _vertPosData);
			_tanVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount*sizeof(VertexTanData), _vertTanData);
			_staticVBOHandle = g_OGLDiv->buffManaRef()->createVertexBuffer(_vertCount*sizeof(VertexStaticData), _vertStaticData);
		}

		return true;
	}

	void GeometryResource::genGeomResVAO(uint32 shaderHandle)
	{
		_vertAttribList.numAttribs = 8;
		_vertAttribList.attribs[0].locationName = "vertPos";
		_vertAttribList.attribs[0].unitSize = 3;
		_vertAttribList.attribs[0].vboHandle = _posVBOHandle;
		_vertAttribList.attribs[0].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[1].locationName = "normal";
		_vertAttribList.attribs[1].unitSize = 3;
		_vertAttribList.attribs[1].offset = 0;
		_vertAttribList.attribs[1].stride = sizeof(VertexTanData);
		_vertAttribList.attribs[1].vboHandle = _tanVBOHandle;
		_vertAttribList.attribs[1].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[2].locationName = "tangent";
		_vertAttribList.attribs[2].unitSize = 3;
		_vertAttribList.attribs[2].offset = 3;
		_vertAttribList.attribs[2].stride = sizeof(VertexTanData);
		_vertAttribList.attribs[2].vboHandle = _tanVBOHandle;
		_vertAttribList.attribs[2].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[3].locationName = "joints";
		_vertAttribList.attribs[3].unitSize = 4;
		_vertAttribList.attribs[3].offset = 2;
		_vertAttribList.attribs[3].stride = sizeof(VertexStaticData);
		_vertAttribList.attribs[3].vboHandle = _tanVBOHandle;
		_vertAttribList.attribs[3].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[4].locationName = "weights";
		_vertAttribList.attribs[4].unitSize = 4;
		_vertAttribList.attribs[4].offset = 6;
		_vertAttribList.attribs[4].stride = sizeof(VertexStaticData);
		_vertAttribList.attribs[4].vboHandle = _staticVBOHandle;
		_vertAttribList.attribs[4].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[5].locationName = "texCoords0";
		_vertAttribList.attribs[5].unitSize = 2;
		_vertAttribList.attribs[5].offset = 0;
		_vertAttribList.attribs[5].stride = sizeof(VertexStaticData);
		_vertAttribList.attribs[5].vboHandle = _staticVBOHandle;
		_vertAttribList.attribs[5].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[6].locationName = "texCoords1";
		_vertAttribList.attribs[6].unitSize = 2;
		_vertAttribList.attribs[6].offset = 10;
		_vertAttribList.attribs[6].stride = sizeof(VertexStaticData);
		_vertAttribList.attribs[6].vboHandle = _staticVBOHandle;
		_vertAttribList.attribs[6].vboType = GL_ARRAY_BUFFER;

		_vertAttribList.attribs[7].vboHandle = _indexVBOHandle;
		_vertAttribList.attribs[7].vboType = GL_ELEMENT_ARRAY_BUFFER;



		g_OGLDiv->shaderManaRef()->setPragramAttribLoc(shaderHandle, _vertAttribList);
		_geomVAOHandle = g_OGLDiv->buffManaRef()->createVAO(_vertAttribList);
	}

	int GeometryResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case GeometryElemType::GeometryElem:
			return 1;
		default:
			return Resource::getElemCount(elem);
		}
	}

	int GeometryResource::getElemParamI(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case GeometryElemType::GeometryElem:
			switch (param)
			{
			case GeometryElemType::GeoIndexCountI:
				return (int)_indexCount;
			case GeometryElemType::GeoIndices16I:
				return _16BitIndices ? 1 : 0;
			case GeometryElemType::GeoVertexCountI:
				return (int)_vertCount;
			}
			break;
		}
		return Resource::getElemParamI(elem, elemIdx, param);
	}

	void GeometryResource::updateDynaDataToGpu()
	{
		if (_vertPosData != 0x0)
		{
			g_OGLDiv->buffManaRef()->updateBufferData(_posVBOHandle, 0, _vertCount * sizeof(Vec3), _vertPosData);
		}
		if (_vertTanData != 0x0)
		{
			g_OGLDiv->buffManaRef()->updateBufferData(_tanVBOHandle, 0, _vertCount * sizeof(VertexTanData), _vertTanData);
		}
	}

	/*void * GeometryResource::mapStream(int elem, int elemIdx, int stream, bool read, bool write)
	{
		if (read || write)
		{
			mappedWriteStream = -1;
			
			switch (elem)
			{
			default:
				break;
			}
		}
	}

	void GeometryResource::unmapStream()
	{

	}*/

}