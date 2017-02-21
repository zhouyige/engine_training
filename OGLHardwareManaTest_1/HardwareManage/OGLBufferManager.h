#pragma once
#include "LeanUtil.h"
#include "glus.h"
#include <string>

namespace Lean3D
{
	

	class OGLBufferManager
	{
	public:
		OGLBufferManager();
		~OGLBufferManager();

		uint32 createVAO(VertexAttribLayoutList &vaoLayoutList);
		uint32 createVertexBuffer(uint32 size, const void *data, uint16 drawtype=GL_DYNAMIC_DRAW);
		uint32 createIndexBuffer(uint32 size, const void *data, uint16 drawtype = GL_DYNAMIC_DRAW);
		void updateBufferData(uint32 bufHanle, uint32 offset, uint32 size, void *data);
		void destroyBuffer(uint32 bufHandle);
		void destroyVAO(uint32 vaoHandle);
		void setCurrentVAO(uint32 vaoHandle);
		void setCurrentVertexBuffer(uint32 bufHanle);
		void setCurrentIndexBuffer(uint32 bufHandle);
		uint32 bufferTotalMem() { return _bufferTotalMem; }

	private:
		uint32 _bufferTotalMem;
		VertexAttribLayoutList                _curVertexLayout;
		ReferenceObjList< VIBuffer >        _buffersReflist;
		ReferenceObjList< VaoBuffer > _vaoReflist;

	};



}