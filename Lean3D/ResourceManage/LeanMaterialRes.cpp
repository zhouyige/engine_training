#include "LeanMaterialRes.h"
#include "LeanResourceManager.h"
#include "utXML.h"

namespace Lean3D
{

	MaterialResource::MaterialResource(const std::string &name, int flags)
		:Resource(ResourceTypes::Material, name, flags)
	{
		initDefault();
	}

	MaterialResource::~MaterialResource()
	{
		release();
	}

	Lean3D::Resource * MaterialResource::clone()
	{
		MaterialResource *res = new MaterialResource("", _flags);
		*res = *this;

		return res;
	}

	void MaterialResource::initDefault()
	{
		_shaderRes = 0x0;
		_combMask = 0;
		_matLink = 0x0;
		_class = "";
	}

	void MaterialResource::release()
	{
		_shaderRes = 0x0;
		_matLink = 0x0;
		for (uint32 i = 0; i < _samplers.size(); ++i)
			_samplers[i].texRes = 0x0;
		_samplers.clear();
		_uniforms.clear();
		_shaderFlags.clear();
	}

	bool MaterialResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size)) return false;

		XMLDoc doc;
		doc.parseBuffer(data, size);
		if (doc.hasError())
		{
			LEAN_DEGUG_LOG("XML 文件格式错误！", 0);
			return false;
		}
		XMLNode rootNode = doc.getRootNode();
		if (strcmp(rootNode.getName(), "Material") != 0)
		{
			LEAN_DEGUG_LOG("非材质文件节点！", 0);
			return false;
		}

		//class 
		_class = rootNode.getAttribute("class", "");

		if (strcmp(rootNode.getAttribute("link", ""), "") != 0)
		{
			uint32 matrialhandle = LeanRoot::resMana().addResource(ResourceTypes::Material
				, std::string(rootNode.getAttribute("link"))
				, 0, false);
			_matLink = (MaterialResource*)LeanRoot::resMana().resolveResHandle(matrialhandle);
			if (_matLink == this)
			{
				LEAN_DEGUG_LOG("error 不能连接材质本身！", 0);
				return false;
			}
		}

		//设置shader flag,控制sheder宏定义
		XMLNode node1 = rootNode.getFirstChild("ShaderFlag");
		while (!node1.isEmpty())
		{
			if (node1.getAttribute("name") == 0x0)
			{
				LEAN_DEGUG_LOG("error ShaderFlag未定义属性名！", 0);
				return false;
			}
			_shaderFlags.push_back(node1.getAttribute("name"));
			node1 = node1.getNextSibling("ShaderFlag");
		}

		//指定Shader
		node1 = rootNode.getFirstChild("Shader");
		if (!node1.isEmpty())
		{
			if (node1.getAttribute("source") == 0x0)
			{
				LEAN_DEGUG_LOG("error 未定义Shader文件路径！", 0);
				return false;
			}
			uint32 shader = LeanRoot::resMana().addResource(ResourceTypes::Shader
				, std::string(node1.getAttribute("source"))
				, 0, false);
			_shaderRes = (ShaderResource*)LeanRoot::resMana().resolveResHandle(shader);
			_shaderRes->setPrefixMacros(_shaderFlags);
			//_combMask = ShaderResource::calcCombMask(_shaderFlags);
			//_shaderRes->preLoadCombination(_combMask);//插入combMask
		}

		//纹理资源
		node1 = rootNode.getFirstChild("Sampler");
		while (!node1.isEmpty())
		{
			if (node1.getAttribute("name") == 0x0)
			{
				LEAN_DEGUG_LOG("error 未定义纹理资源name", 0);
				return false;
			}
			if (node1.getAttribute("map") == 0x0)
			{
				LEAN_DEGUG_LOG("error 未定义纹理资源路径！", 0);
				return false;
			}
			MatSampler sampler;
			sampler.name = node1.getAttribute("name");

			ResHandle texMap;
			uint32 flags = 0;
			///if (!LeanRoot::)
			//ASSERT(0);

			if (_stricmp(node1.getAttribute("allowCompression", "true"), "false") == 0 ||
				_stricmp(node1.getAttribute("allowCompression", "1"), "0") == 0)
				flags |= ResourceFlags::NoTexCompression;

			if (_stricmp(node1.getAttribute("mipmaps", "true"), "false") == 0 ||
				_stricmp(node1.getAttribute("mipmaps", "1"), "0") == 0)
				flags |= ResourceFlags::NoTexMipmaps;

			if (_stricmp(node1.getAttribute("sRGB", "false"), "true") == 0 ||
				_stricmp(node1.getAttribute("sRGB", "0"), "1") == 0)
				flags |= ResourceFlags::TexSRGB;

			texMap = LeanRoot::resMana().addResource(ResourceTypes::Texture
				, std::string(node1.getAttribute("map"))
				, flags, false);
			sampler.texRes = (TextureResource*)LeanRoot::resMana().resolveResHandle(texMap);
			_samplers.push_back(sampler);
			node1 = node1.getNextSibling("Sampler");
		}

		//矢量一致变量
		node1 = rootNode.getFirstChild("Uniform");
		while (!node1.isEmpty())
		{
			if (node1.getAttribute("name") == 0x0)
			{
				LEAN_DEGUG_LOG("error 未定义uniform一致变量 name！", 0);
				return false;
			}

			MatUniform uniform;
			uniform.name = node1.getAttribute("name");
			uniform.values[0] = (float)atof(node1.getAttribute("a", "0"));
			uniform.values[1] = (float)atof(node1.getAttribute("b", "0"));
			uniform.values[2] = (float)atof(node1.getAttribute("c", "0"));
			uniform.values[3] = (float)atof(node1.getAttribute("d", "0"));

			_uniforms.push_back(uniform);
			node1 = node1.getNextSibling("Uniform");
		}

		return true;
	}

	bool MaterialResource::setUniform(const std::string &name, float a, float b, float c, float d)
	{
		for (uint32 i = 0; i < _uniforms.size(); ++i)
		{
			if (_uniforms[i].name == name)
			{
				_uniforms[i].values[0] = a;
				_uniforms[i].values[1] = b;
				_uniforms[i].values[2] = c;
				_uniforms[i].values[3] = d;
				return true;
			}
		}
		return false;
	}

	bool MaterialResource::isOfClass(const std::string &theClass)
	{
		static std::string theClass2;

		if (theClass != "")
		{
			if (theClass[0] != '~')
			{
				if (_class.find(theClass, 0) != 0) return false;
				if (_class.length() > theClass.length() && _class[theClass.length()] != '.')
					return false;
			}
			else
			{
				theClass2 = theClass.substr(1, theClass.length() - 1);
				if (_class.find(theClass2, 0) == 0)
				{
					if (_class.length() == theClass2.length())
						return false;
					else
						if (_class[theClass2.length()] == '.')
							return false;
				}
			}
		}
		else
		{
			if (_class == "_DEBUG_") return false;
		}
		return true;
	}

	int MaterialResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case MaterialElemType::MaterialElem:
			return 1;
		case MaterialElemType::SamplerElem:
			return (int)_samplers.size();
		case MaterialElemType::UniformElem:
			return (int)_uniforms.size();
		default:
			return Resource::getElemCount(elem);
		}
	}

	int MaterialResource::getElemParamI(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case MaterialElemType::MaterialElem:
			switch (param)
			{
			case MaterialElemType::MatLinkI:
				return _matLink != 0x0 ? _matLink->getHandle() : 0;
			case MaterialElemType::MatShaderI:
				return _shaderRes != 0x0 ? _shaderRes->getHandle() : 0;
			}
			break;
		case MaterialElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case  MaterialElemType::SampTexResI:
					return _samplers[elemIdx].texRes->getHandle();
				}
			}
			break;
		}
		return Resource::getElemParamI(elem, elemIdx, param);
	}

	void MaterialResource::setElemParamI(int elem, int elemIdx, int param, int value)
	{
		switch (elem)
		{
		case MaterialElemType::MaterialElem:
			switch (param)
			{
			case MaterialElemType::MatLinkI:
				if (value == 0)
				{
					_matLink = 0x0;
					return;
				}
				else
				{
					Resource *res = LeanRoot::resMana().resolveResHandle(value);
					if (res != 0x0 && res->getType() == ResourceTypes::Material)
						_matLink = (MaterialResource *)res;
					else
					{
						LEAN_DEGUG_LOG("error 设置link材质读取res出错！", 0);
						return ;
					}
					return;
				}
				break;
			case MaterialElemType::MatShaderI:
				if (value == 0)
				{
					_shaderRes = 0x0;
					return;
				}
				else
				{
					Resource *res = LeanRoot::resMana().resolveResHandle(value);
					if (res != 0x0 && res->getType() == ResourceTypes::Shader)
						_shaderRes = (ShaderResource *)res;
					else
					{
						LEAN_DEGUG_LOG("error 设置材质绑定shader读取res出错！", 0);
						return;
					}
					return;
				}
				break;
			}
			break;
		case MaterialElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case MaterialElemType::SampTexResI:
					Resource *res = LeanRoot::resMana().resolveResHandle(value);
					if (res != 0x0 && res->getType() == ResourceTypes::Texture)
						_samplers[elemIdx].texRes = (TextureResource *)res;
					else
					{
						LEAN_DEGUG_LOG("error 设置材质sampler读取res出错！", 0);
						return;
					}
					return;
				}
			}
			break;
		}

		Resource::setElemParamI(elem, elemIdx, param, value);
	}

	float MaterialResource::getElemParamF(int elem, int elemIdx, int param, int compIdx)
	{
		switch (elem)
		{
		case MaterialElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case MaterialElemType::UnifValueF4:
					if ((unsigned)compIdx < 4) 
						return _uniforms[elemIdx].values[compIdx];
					break;
				}
			}
			break;
		}

		return Resource::getElemParamF(elem, elemIdx, param, compIdx);
	}

	void MaterialResource::setElemParamF(int elem, int elemIdx, int param, int compIdx, float value)
	{
		switch (elem)
		{
		case MaterialElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case MaterialElemType::UnifValueF4:
					if ((unsigned)compIdx < 4)
					{
						_uniforms[elemIdx].values[compIdx] = value;
						return;
					}
					break;
				}
			}
			break;
		}

		Resource::setElemParamF(elem, elemIdx, param, compIdx, value);
	}

	const char * MaterialResource::getElemParamStr(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case MaterialElemType::MaterialElem:
			switch (param)
			{
			case MaterialElemType::MatClassStr:
				return _class.c_str();
			}
			break;
		case MaterialElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case MaterialElemType::SampNameStr:
					return _samplers[elemIdx].name.c_str();
				}
			}
			break;
		case MaterialElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case MaterialElemType::UnifNameStr:
					return _uniforms[elemIdx].name.c_str();
				}
			}
			break;
		}

		return Resource::getElemParamStr(elem, elemIdx, param);
	}

	void MaterialResource::setElemParamStr(int elem, int elemIdx, int param, const char *value)
	{
		switch (elem)
		{
		case MaterialElemType::MaterialElem:
			switch (param)
			{
			case MaterialElemType::MatClassStr:
				_class = value;
				return;
			}
			break;
		}

		Resource::setElemParamStr(elem, elemIdx, param, value);
	}

}