#pragma once
#include "LeanUtil.h"
#include "LeanResource.h"

namespace Lean3D
{
	struct AnimationElemType
	{
		enum List
		{
			AnimaElem = 1100,
			FrameCountI
		};

	};

	//存储每帧旋转，位移，缩放并保存预先将所有变换计算为变换矩阵
	struct Frame
	{
		Quaternion  rotQuat;
		Vec3		transVec;
		Vec3		scaleVec;
		Matrix4		precompMat;
	};

	//每个顶点或关节一次动画变换的一个路径每个阶段
	struct AnimaPhase
	{
		uint32				ID;
		Matrix4				firstFrameInvTrans;
		std::vector<Frame>  frames;
	};
	//
	class AnimationResource : public Resource
	{
	public:
		AnimationResource(const std::string &name, int flags);
		~AnimationResource();
		Resource *clone();

		static Resource *factoryFunc(const std::string &name, int flags)
		{
			return  new AnimationResource(name, flags);
		}
		void initDefault();
		void release();
		bool load(const char *data, int size);

		int getElemCount(int elem);
		int getElemParamI(int elem, int elemIdx, int param);

		AnimaPhase *findAnimaPhase(uint32 id);

	private:
		uint32 _frameCount;
		std::vector< AnimaPhase > _phases;
	};


}