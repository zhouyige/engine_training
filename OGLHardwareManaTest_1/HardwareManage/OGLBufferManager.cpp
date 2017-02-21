#include "OGLBufferManager.h"

namespace Lean3D
{
	OGLBufferManager::OGLBufferManager()
	{
	}

	OGLBufferManager::~OGLBufferManager()
	{
		
	}

	uint32 OGLBufferManager::createVAO(VertexAttribLayoutList &vaoLayoutList)
	{
		VaoBuffer vaobuf;
		vaobuf.vaoRef = -1;

		glGenVertexArrays(1, &vaobuf.vaoRef);
		glBindVertexArray(vaobuf.vaoRef);

		for (int i = 0; i < vaoLayoutList.numAttribs; ++i)
		{
			VertexAttribLayout &crrentAttrib = vaoLayoutList.attribs[i];
			VIBuffer &vbobuf = _buffersReflist.getRef(crrentAttrib.vboHandle);

			if (crrentAttrib.vboType == GL_ARRAY_BUFFER)
			{
				glBindBuffer(crrentAttrib.vboType, vbobuf.bufRef);
				glVertexAttribPointer(crrentAttrib.attribLoc, crrentAttrib.unitSize, GL_FLOAT, GL_FALSE, crrentAttrib.stride, (char*)0 + crrentAttrib.offset);
				glEnableVertexAttribArray(crrentAttrib.attribLoc);
			}
			else if (crrentAttrib.vboType == GL_ELEMENT_ARRAY_BUFFER)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbobuf.bufRef);
			}
			else
			{
				ASSERT(0);
			}
		}
		glBindVertexArray(0);
		
		return _vaoReflist.add(vaobuf);;
	}

	uint32 OGLBufferManager::createVertexBuffer(uint32 size, const void *data, uint16 drawtype/*=GL_DYNAMIC_DRAW*/)
	{
		VIBuffer vbobuf;

		vbobuf.bufferType = GL_ARRAY_BUFFER;
		vbobuf.bufferSize = size;
		vbobuf.drawType = drawtype;
		glGenBuffers(1, &vbobuf.bufRef);
		glBindBuffer(vbobuf.bufferType, vbobuf.bufRef);
		glBufferData(vbobuf.bufferType, size, data, drawtype);
		glBindBuffer(vbobuf.bufferType, 0);
		_bufferTotalMem += size;

		return _buffersReflist.add(vbobuf);
	}

	uint32 OGLBufferManager::createIndexBuffer(uint32 size, const void *data, uint16 drawtype/*=GL_DYNAMIC_DRAW*/)
	{
		VIBuffer idxbuf;

		idxbuf.bufferType = GL_ELEMENT_ARRAY_BUFFER;
		idxbuf.bufferSize = size;
		idxbuf.drawType = drawtype;
		glGenBuffers(1, &idxbuf.bufRef);
		glBindBuffer(idxbuf.bufferType, idxbuf.bufRef);
		glBufferData(idxbuf.bufferType, size, data, drawtype);
		glBindBuffer(idxbuf.bufferType, 0);
		_bufferTotalMem += size;

		return _buffersReflist.add(idxbuf);
	}

	void OGLBufferManager::updateBufferData(uint32 bufHandle, uint32 offset, uint32 size, void *data)
	{
		const VIBuffer &buf = _buffersReflist.getRef(bufHandle);
		ASSERT(offset + size <= buf.bufferSize);

		glBindBuffer(buf.bufferType, buf.bufRef);
		if (offset ==0 && size == buf.bufferSize)
		{
			glBufferData(buf.bufferType, size, data, buf.drawType);
			return;
		}
		glBufferSubData(buf.bufferType, offset, size, data);
	}

	void OGLBufferManager::destroyBuffer(uint32 bufHandle)
	{
		if (bufHandle == 0) return;

		VIBuffer &buf = _buffersReflist.getRef(bufHandle);
		glDeleteBuffers(1, &buf.bufRef);
		_bufferTotalMem -= buf.bufferSize;
		_buffersReflist.remove(bufHandle);
	}

	void OGLBufferManager::destroyVAO(uint32 vaoHandle)
	{
		if (vaoHandle == 0) return;
		
		VaoBuffer &vaobuf = _vaoReflist.getRef(vaoHandle);
		glDeleteVertexArrays(1, &vaobuf.vaoRef);
		_vaoReflist.remove(vaoHandle);
	}

	void OGLBufferManager::setCurrentVAO(uint32 vaoHandle)
	{
		if(vaoHandle == 0) return;

		VaoBuffer &vaobuf = _vaoReflist.getRef(vaoHandle);
		glBindVertexArray(vaobuf.vaoRef);
	}

	void OGLBufferManager::setCurrentVertexBuffer(uint32 bufHandle)
	{
		if (bufHandle == 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			return;
		}
		VIBuffer &vbobuf = _buffersReflist.getRef(bufHandle);
		glBindBuffer(vbobuf.bufferType, vbobuf.bufRef);
		
	}

	void OGLBufferManager::setCurrentIndexBuffer(uint32 bufHandle)
	{
		if (bufHandle == 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			return;
		}
		VIBuffer &idxbuf = _buffersReflist.getRef(bufHandle);
		glBindBuffer(idxbuf.bufferType, idxbuf.bufRef);
	}


}