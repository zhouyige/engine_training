#include "LeanAnimationRes.h"
#include <algorithm>

namespace Lean3D
{
	AnimationResource::AnimationResource(const std::string &name, int flags) 
		: Resource(ResourceTypes::Animation, name, flags)
	{
		initDefault();
	}

	AnimationResource::~AnimationResource()
	{
		release();
	}
	Resource * Lean3D::AnimationResource::clone()
	{
		AnimationResource *res = new AnimationResource("", _flags);
		*res = *this;

		return res;
	}

	void Lean3D::AnimationResource::initDefault()
	{
		_frameCount = 0;
	}

	void Lean3D::AnimationResource::release()
	{
		_phases.clear();
	}

	bool Lean3D::AnimationResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size)) return false;
		
		if (size < 8)
		{
			LEAN_DEGUG_LOG("Error 错误的animation文件！", 0);
			return false;
		}

		char *pData = (char*)data;

		char header[4];
		memcpy(&header, pData, 4);
		pData += 4;
		if (header[0] != 'H' || header[1] != '3' || header[2] != 'D' || header[3] != 'A')
		{
			LEAN_DEGUG_LOG("Error 错误的animation文件！", 0);
			return false;
		}
		uint32 version;
		memcpy(&version, pData, sizeof(uint32));
		pData += sizeof(uint32);
		if (version != 1 && version != 2 && version != 3)
		{
			LEAN_DEGUG_LOG("Error 错误的animation文件！", 0);
			return false;
		}

		//读取动画数据
		uint32 phaseCount;
		memcpy(&phaseCount, pData, sizeof(uint32));
		pData += sizeof(uint32);
		memcpy(&_frameCount, pData, sizeof(uint32));
		pData += sizeof(uint32);

		_phases.resize(phaseCount);

		for (uint32 i = 0; i < phaseCount; ++i)
		{
			char name[256], compressed = 0;
			AnimaPhase &phase = _phases[i];

			memcpy(name, pData, 256);
			pData += 256;
			// SDBM hash
			uint32 hash = 0;
			char *pname = name;
			while (char ch = *pname++)
				hash = ch + (hash << 6) + (hash << 16) - hash;
			phase.ID = hash;

			//若是版本3 数据被压缩
			if (version == 3)
			{
				memcpy(&compressed, pData, sizeof(char));
				pData += sizeof(char);
			}

			phase.frames.resize(compressed ? 1 : _frameCount);
			for (uint32 j = 0; j < (compressed ? 1 : _frameCount); ++j)
			{
				Frame &frame = phase.frames[j];

				memcpy(&frame.rotQuat.x, pData, sizeof(float));
				pData += sizeof(float);
				memcpy(&frame.rotQuat.y, pData, sizeof(float));
				pData += sizeof(float);
				memcpy(&frame.rotQuat.z, pData, sizeof(float));
				pData += sizeof(float);
				memcpy(&frame.rotQuat.w, pData, sizeof(float));
				pData += sizeof(float);

				memcpy(&frame.transVec.x, pData, sizeof(float)); 
				pData += sizeof(float);
				memcpy(&frame.transVec.y, pData, sizeof(float)); 
				pData += sizeof(float);
				memcpy(&frame.transVec.z, pData, sizeof(float)); 
				pData += sizeof(float);

				memcpy(&frame.scaleVec.x, pData, sizeof(float)); 
				pData += sizeof(float);
				memcpy(&frame.scaleVec.y, pData, sizeof(float)); 
				pData += sizeof(float);
				memcpy(&frame.scaleVec.z, pData, sizeof(float)); 
				pData += sizeof(float);

				//提前计算变换矩阵
				frame.precompMat.scale(frame.scaleVec.x, frame.scaleVec.y, frame.scaleVec.z);
				frame.precompMat = Matrix4(frame.rotQuat) * frame.precompMat;
				frame.precompMat.translate(frame.transVec.x, frame.transVec.y, frame.transVec.z);
			}
			//保存第一帧的逆变矩阵
			if (!phase.frames.empty())
				phase.firstFrameInvTrans = phase.frames[0].precompMat.inverted();
		}

		//根据SDBM哈希id升序排序
		std::sort(_phases.begin(), _phases.end(), [](AnimaPhase &p1, AnimaPhase &p2){return p1.ID < p2.ID;});
	
		return true;
	}

	int Lean3D::AnimationResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case AnimationElemType::AnimaElem:
			return (int)_phases.size();
		default:
			return Resource::getElemCount(elem);
		}
	}

	int Lean3D::AnimationResource::getElemParamI(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case AnimationElemType::AnimaElem:
			switch (param)
			{
			case AnimationElemType::FrameCountI:
				return _frameCount;
			}
			break;
		}
		return Resource::getElemParamI(elem, elemIdx, param);
	}

	AnimaPhase * Lean3D::AnimationResource::findAnimaPhase(uint32 id)
	{
		//二叉查找
		int first = 0, last = (int)_phases.size() - 1;
		while (first <= last)
		{
			int mid = (first + last) / 2;
			if (id > _phases[mid].ID)
				first = mid + 1;
			else if (id < _phases[mid].ID)
				last = mid - 1;
			else
				return &_phases[mid];
		}
		return 0x0;
	}



}


