#include "OGLRenderBase.h"


namespace Lean3D
{
	Lean3D::OGLRenderBase::OGLRenderBase()
	{

	}

	void OGLRenderBase::initStates()
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//���ض���Ϊ1�ı���
	}

	bool OGLRenderBase::init()
	{
		//�˴�Ӧ�ü�鱾����OPENGL��֧��״̬
		//����ֻ�Ǵ�ų�ʼ���� 
		_caps.texFloat = true;
		_caps.texNPOT = true;
		_caps.rtMultisampling = true;
		
		//��Ȼ����ʽ  ������ati�Կ�ֻ֧��16λ��Ȼ���FBO  �ҽ��������
		_depthFormat = GL_DEPTH_COMPONENT24; 
		
		initStates();

	}

	uint32 OGLRenderBase::createVAO(VertexAttribList vaoLayoutList)
	{

	}

}


