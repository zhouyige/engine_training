#pragma once

#include "utMath.h"

namespace Lean3D
{
	//****************************************//
	//AABB绑定盒
	//****************************************//

	enum BdBoxCorner
	{
		BOX_FLB = 0,	//box 正左下点
		BOX_FRB,		//box 正右下点
		BOX_FRT,		//box 正右上点
		BOX_FLT,		//box 正左上点
		BOX_BLB,		//box 背左下点
		BOX_BRB,		//box 背右下点
		BOX_BRT,		//box 背右上点
		BOX_BLT			//box 背左上点
	};
	struct BoundingBox
	{
		Vec3 min, max;

		inline void zero()
		{
			min = Vec3(0, 0, 0);
			max = Vec3(0, 0, 0);
		}
		
		inline Vec3 getCorner(BdBoxCorner corner) const
		{
			switch (corner)
			{
			case Lean3D::BOX_FLB:
				return Vec3(min.x, min.y, max.z);
			case Lean3D::BOX_FRB:
				return Vec3(max.x, min.y, max.z);
			case Lean3D::BOX_FRT:
				return Vec3(max.x, max.y, max.z);
			case Lean3D::BOX_FLT:
				return Vec3(min.x, max.y, max.z);
			case Lean3D::BOX_BLB:
				return Vec3(min.x, min.y, min.z);
			case Lean3D::BOX_BRB:
				return Vec3(max.x, min.y, min.z);
			case Lean3D::BOX_BRT:
				return Vec3(max.x, max.y, min.z);
			case Lean3D::BOX_BLT:
				return Vec3(min.x, max.y, min.z);
			default:
				return Vec3();
			}
		}

		void transform(const Matrix4 &m)
		{
			//图形图像精粹上的高效 AABB变换 算法

			float minA[3] = { min.x, min.y, min.z }, minB[3];
			float maxA[3] = { max.x, max.y, max.z }, maxB[3];

			for (unsigned int i = 0; i < 3; ++i)
			{
				minB[i] = m.c[3][i];
				maxB[i] = m.c[3][i];

				for (unsigned int j = 0; j < 3; ++j)
				{
					float x = minA[j] * m.c[j][i];
					float y = maxA[j] * m.c[j][i];
					minB[i] += minf(x, y);
					maxB[i] += maxf(x, y);
				}
			}

			min = Vec3(minB[0], minB[1], minB[2]);
			max = Vec3(maxB[0], maxB[1], maxB[2]);
		}

		//将两个box合并为一个包含两个box的box
		bool makeUnion(BoundingBox &b)
		{
			bool changed = false;

			// 忽略无效box
			if (min == max)
			{
				changed = true;
				min = b.min;
				max = b.max;
			}
			else if (b.min != b.max)
			{
				if (b.min.x < min.x) { changed = true; min.x = b.min.x; }
				if (b.min.y < min.y) { changed = true; min.y = b.min.y; }
				if (b.min.z < min.z) { changed = true; min.z = b.min.z; }

				if (b.max.x > max.x) { changed = true; max.x = b.max.x; }
				if (b.max.y > max.y) { changed = true; max.y = b.max.y; }
				if (b.max.z > max.z) { changed = true; max.z = b.max.z; }
			}

			return changed;
		}
	};

	//****************************************//
	//视锥体
	//****************************************//

}