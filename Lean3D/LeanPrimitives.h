#pragma once

#include "utMath.h"

namespace Lean3D
{
	//****************************************//
	//AABB�󶨺�
	//****************************************//

	enum BdBoxCorner
	{
		BOX_FLB = 0,	//box �����µ�
		BOX_FRB,		//box �����µ�
		BOX_FRT,		//box �����ϵ�
		BOX_FLT,		//box �����ϵ�
		BOX_BLB,		//box �����µ�
		BOX_BRB,		//box �����µ�
		BOX_BRT,		//box �����ϵ�
		BOX_BLT			//box �����ϵ�
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
			//ͼ��ͼ�񾫴��ϵĸ�Ч AABB�任 �㷨

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

		//������box�ϲ�Ϊһ����������box��box
		bool makeUnion(BoundingBox &b)
		{
			bool changed = false;

			// ������Чbox
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
	//��׶��
	//****************************************//

}