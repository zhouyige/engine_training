#include "LeanShaderRes.h"
#include "Lean3DRoot.h"
#include "LeanResourceManager.h"

namespace Lean3D
{


	CodeResource::CodeResource(const std::string &name, int flags)
				: Resource(ResourceTypes::Code, name , flags)
	{
		initDefault();
	}

	CodeResource::~CodeResource()
	{
		release();
	}

	Resource * CodeResource::clone()
	{
		CodeResource *res = new CodeResource("", _flags);

		*res = *this;

		return res;
	}

	void CodeResource::initDefault()
	{
		_flagMask = 0;
		_code.clear();
	}

	void CodeResource::release()
	{
		for (uint32 i = 0; i < _include.size(); ++i)
		{
			_include[i].first = 0x0;
		}

		_include.clear();
	}

	bool CodeResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size))  return false;

		char *code = new char[size + 1];
		char *pCode = code;
		const char *pData = data;
		const char *eof = data + size;

		bool lineComment = false, blockComment = false;

		while (pData < eof)
		{
			//检查注释标志开始
			if (pData < eof	- 1 && !lineComment && !blockComment)
			{
				if (*pData == '/' && *(pData + 1) == '/')
					lineComment = true;
				else if (*pData == '/' && *(pData + 1) == '*')
					blockComment = true;
			}

			//检查注释标注结束
			if (lineComment && (*pData == '\n' || *pData == '\r'))
				lineComment = false;
			else if (blockComment && pData < eof - 1 && *pData == '*' && *(pData + 1) == '/')
				blockComment = false;

			if ( !lineComment && !blockComment && pData < eof - 7)
			{
				if (*pData == '#' && *(pData+1) == 'i' && *(pData+2) == 'n' && *(pData+3) == 'c' &&
					*(pData+4) == 'l' && *(pData+5) == 'u' && *(pData+6) == 'd' && *(pData+7) == 'e')
				{
					pData += 6;

					//解析include的文件路径+文件名
					const char *nameBeigin = 0x0, *nameEnd = 0x0;

					while (++pData < eof)
					{
						if (*pData == '"')
						{
							if (nameBeigin == 0x0)
								nameBeigin = pData + 1;
							else
								nameEnd = pData;
						}
						else if (*pData == '\n' || *pData == '\r')
							break;

					}
					if (nameBeigin != 0x0 && nameEnd != 0x0)
					{
						std::string resName(nameBeigin, nameEnd);
						ResHandle handle =  LeanRoot::resMana().addResource(ResourceTypes::Code, resName, 0, false);
						CodeResource *codeRes = (CodeResource*)LeanRoot::resMana().resolveResHandle(handle);
						//CodeResource *codeRes =  (CodeResource*)CodeResource::factoryFunc(resName, 0);
						_include.push_back(std::pair<ReferenceCountPtr<CodeResource>, size_t>(codeRes, pCode - code));

					}
					else
					{
						delete[] code;
						LEAN_DEGUG_LOG("错误#include shader代码！", 0);
						return false;
					}
				}
			}

			//检查flags ， 设置宏定义控制引入的着色器代码段
			if (!lineComment && !blockComment && pData < eof - 4)
			{
				if (*pData == '_' && *(pData+1) == 'F' && *(pData+4) == '_' &&
					*(pData+2) >= 48 && *(pData+2) <= 57 && *(pData+3) >= 48 && *(pData+3) <= 57)
				{
					uint32 flagvalue = (*(pData+2) - 48) * 10 + (*(pData+3) - 48);
					_flagMask |= 1 << (flagvalue - 1);
					for (uint32 i = 0; i < 5; ++i)
						*pCode++ = *pData++;
					//忽略flag后面的提示字符
					while (pData < eof && *pData != ' ' && *pData != '\t' && *pData != '\n' && *pData != '\r')
						++pData;
				}

			}

			*pCode++ = *pData++;
		}

		*pCode = '\0';
		_code = code;
		delete[] code;
		code = 0x0;
		//因为include的文件并未load，所以contextcompile不会通过
		//当每读取一次codeResource检查include是否读取完整
		//一段完整shader代码读取成功，则调用contextCompile成功
		updateShaders();

		return true;
	}

	bool CodeResource::hasDependency(CodeResource *codeRes)
	{
		if (codeRes == this) return true;
		for (uint32 i = 0; i < _include.size(); ++i)
		{
			if (_include[i].first->hasDependency(codeRes))
				return true;
		}
		return false;
	}

	bool CodeResource::getCodeLinkFlags(uint32 *flagMask)
	{
		//检查相关联的code文件是否都被load
		if (!_loaded) return false;
		if (flagMask != 0x0) *flagMask |= _flagMask;
		for (uint32 i = 0; i < _include.size(); ++i)
		{
			if (!_include[i].first->getCodeLinkFlags(flagMask))
				return false;
		}

		return true;
	}

	std::string CodeResource::assembleCode()
	{
		if (!_loaded) return "";

		std::string finalCode = _code;
		uint32 offset = 0;

		for (uint32 i = 0; i < _include.size(); ++i)
		{
			std::string includeCode = _include[i].first->assembleCode();
			finalCode.insert(_include[i].second + offset, includeCode);
			offset += (uint32)includeCode.length();
		}

		return finalCode;
	}

	void CodeResource::updateShaders()
	{
		//2016/05/24 add by zhou
		for (uint32 i = 0; i < LeanRoot::resMana().getResourceList().size(); ++i)
		{
			Resource *res = LeanRoot::resMana().getResourceList()[i];
			if (res != 0x0 && res->getType() == ResourceTypes::Shader)
			{
				ShaderResource *shaderRes = (ShaderResource*)res;
				for (uint32 j = 0; j < shaderRes->getContexts().size(); ++j)
				{
					ShaderContext &context = shaderRes->getContexts()[j];
					if (shaderRes->getCode(context.vertCodeIdx)->hasDependency(this)
						|| shaderRes->getCode(context.fragCodeIdx)->hasDependency(this))
					{
						context.compiled = false;
					}
					else if ((context.geoCodeIdx != -1 && shaderRes->getCode(context.geoCodeIdx)->hasDependency(this))
						|| (context.TescontCodeIdx != -1 && shaderRes->getCode(context.TescontCodeIdx)->hasDependency(this))
						|| (context.TesevalCodeIdx != -1 && shaderRes->getCode(context.TesevalCodeIdx)->hasDependency(this)))
					{
						context.compiled = false;
					}
				}
				shaderRes->compileContexts();
			}
		}
	}

	std::string ShaderResource::_vertPreamble = "#version 110 core\n";
	std::string ShaderResource::_fragPreamble = "#version 110 core\n";
	std::string ShaderResource::_geoPreamble = "#version 150 core\n";
	std::string ShaderResource::_tescontPreamble = "#version 150 core\n";
	std::string ShaderResource::_tesevalPreamble = "#version 150 core\n";
	std::string ShaderResource::_tmpCode0 = "";
	std::string ShaderResource::_tmpCode1 = "";
	std::string ShaderResource::_tmpCode2 = "";
	std::string ShaderResource::_tmpCode3 = "";
	std::string ShaderResource::_tmpCode4 = "";

	ShaderResource::ShaderResource(const std::string &name, int flags)
		:Resource(ResourceTypes::Shader, name, flags)
	{
		initDefault();
	}

	ShaderResource::~ShaderResource()
	{
		release();
	}

	void ShaderResource::initDefault()
	{


	}

	void ShaderResource::release()
	{
		for (uint32 i = 0; i < _contexts.size(); ++i)
		{
			for (uint32 j = 0; j < _contexts[i].shaderCombs.size(); ++j)
			{
				g_OGLDiv->shaderManaRef()->destroyShader(_contexts[i].shaderCombs[j].shaderHandle);
			}
		}

		_contexts.clear();
		_samplers.clear();
		_uniforms.clear();
		_codeSections.clear();
	}

	Resource * ShaderResource::clone()
	{
		LEAN_DEGUG_LOG( "ShaderResource未实现clone拷贝！", 0);
		ASSERT(0);
		return nullptr;
	}

	bool ShaderResource::load(const char *data, int size)
	{
		if (!Resource::load(data, size)) return false;

		//
		const char *pData = data;
		const char *eof = data + size;
		char *fxCode = 0x0;

		while (pData < eof)
		{
			if (pData < eof -1 && *pData == '[' && *(pData+1) == '[')
			{
				pData += 2;

				//解析节点名
				const char *sectionNameStart = pData;
				while (pData < eof && *pData != ']' && *pData != '\n' && *pData != '\r')
					++pData;
				const char *sectionNameEnd = pData++;

				if (pData >= eof || *pData++ != ']')
				{
					LEAN_DEGUG_LOG("error shader文件节点名错误！", 0);
					return false;
				}

				const char *sectionContentStart = pData;
				while ((pData < eof && *pData != '[') || (pData < eof - 1 && *(pData + 1) != '['))
					++pData;
				const char *sectionContentEnd = pData;

				if (sectionNameEnd - sectionNameStart == 2 &&
					*sectionNameStart == 'F' && *(sectionNameStart+1) == 'X')
				{
					//FX节点内容保存到fxCode指向的堆内存
					if (fxCode != 0x0)
					{
						LEAN_DEGUG_LOG("error FX节点超过一个！", 0);
						return false;
					}
					fxCode = new char[sectionContentEnd - sectionContentStart+1];
					memcpy(fxCode, sectionContentStart, sectionContentEnd - sectionContentStart);
					fxCode[sectionContentEnd - sectionContentStart] = '\0';
				}
				else
				{
					//
					_tmpCode0.assign(sectionNameStart, sectionNameEnd);
					_codeSections.push_back(CodeResource(_tmpCode0, 0));
					_tmpCode0.assign(sectionContentStart, sectionContentEnd);
					_codeSections.back().load(_tmpCode0.c_str(), (uint32)_tmpCode0.length());
				}
			}
			else
			{
				++pData;
			}
		}

		if (fxCode == 0x0)
		{
			LEAN_DEGUG_LOG("error shader文件没有Fx节点！", 0);
			return false;
		}
		bool result = parseFXSection(fxCode);
		
		delete[] fxCode;
		fxCode = 0x0;
		if (!result) return false;

		compileContexts();

		return true;
	}

	bool ShaderResource::parseFXSection(char *data)
	{
		char *pData = data;
		//忽略被注释的文本
		while (*pData)
		{
			if (*pData == '/' && *(pData+1) == '/')
			{
				while (*pData && *pData != '\n' && *pData != '\r')
					*pData++ = ' ';
				if (*pData == '\0') break;
			}
			else if (*pData == '/' && *(pData+1) == '*')
			{
				*pData++ = ' '; *pData++ = ' ';
				while (*pData && (*pData != '*' || *(pData + 1) != '/'))
					*pData++ = ' ';
				if (*pData == '\0')
				{
					LEAN_DEGUG_LOG("error Fx没有*/！", 0);
					return false;
				}
				*pData++ = ' '; *pData++ = ' ';
			}
			++pData;
		}
		pData = 0x0;
		const char *identifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
		const char *intnum = "+-0123456789";
		const char *floatnum = "+-0123456789.eE";

		bool unitFree[12] = { true, true, true, true, true, true, true, true, true, true, true, true };

		Tokenizer tok(data);

		while (tok.hasToken())
		{
			if (tok.checkToken("float"))
			{
				ShaderUniform unifrom;
				unifrom.size = 1;
				unifrom.id = tok.getToken(identifier);
				if (unifrom.id == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
				unifrom.defValues[0] = unifrom.defValues[1] = unifrom.defValues[2] = unifrom.defValues[3] = 0.0f;
				if (tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (tok.checkToken("="))
					unifrom.defValues[0] = (float)atof(tok.getToken(floatnum));
				if (!tok.checkToken(";"))
				{
					LEAN_DEGUG_LOG("error Fx缺少 ; 在第%d行！", tok.getLine());
					return false;
				}
				_uniforms.push_back(unifrom);		
			}
			else if (tok.checkToken("float4"))
			{
				ShaderUniform uniform;
				uniform.size = 4;
				uniform.id = tok.getToken(identifier);
				if (uniform.id == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
				uniform.defValues[0] = uniform.defValues[1] = uniform.defValues[2] = uniform.defValues[3] = 0.0f;
				//跳过<>注释
				if (tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (tok.checkToken("="))
				{
					if (!tok.checkToken("{"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 { 在第%d行！", tok.getLine());
						return false;
					}
					uniform.defValues[0] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform.defValues[1] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform.defValues[2] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform.defValues[3] = (float)atof(tok.getToken(floatnum));
					if (!tok.checkToken("}"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 } 在第%d行！", tok.getLine());
						return false;
					}
				}
				if (!tok.checkToken(";"))
				{
					LEAN_DEGUG_LOG("error Fx缺少 ; 在第%d行！", tok.getLine());
					return false;
				}
				_uniforms.push_back(uniform);
			}
			else if (tok.checkToken("sampler2D", true) || tok.checkToken("samplerCube", true) 
					|| tok.checkToken("sampler3D", true))
			{
				ShaderSampler sampler;
				sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;
				if (tok.checkToken("sampler2D"))
				{
					sampler.type = TextureType::Tex2D;
					sampler.defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$Tex2D");
				}
				else if (tok.checkToken("sampler3D"))
				{
					sampler.type = TextureType::Tex3D;
					sampler.defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$Tex3D");
				}
				else if (tok.checkToken("samplerCube"))
				{
					sampler.type = TextureType::TexCube;
					sampler.defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$TexCube");
				}
				sampler.id = tok.getToken(identifier);
				if (sampler.id == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
				if(tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (tok.checkToken("="))
				{
					if (!tok.checkToken("sampler_state"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 sampler_state 在第%d行！", tok.getLine());
						return false;
					}
					if (!tok.checkToken("{"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 { 在第%d行！", tok.getLine());
						return false;
					}
					while (true)
					{
						if (!tok.hasToken())
						{
							LEAN_DEGUG_LOG("error Fx缺少 } 在第%d行！", tok.getLine());
							return false;
						}
						else if (tok.checkToken("}"))
						{
							break;
						}
						else if (tok.checkToken("Texture"))
						{
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							ResHandle texMap = LeanRoot::resMana().addResource(
												ResourceTypes::Texture, std::string(tok.getToken(0x0))
												, 0, false);
							sampler.defTex = (TextureResource *)LeanRoot::resMana().resolveResHandle(texMap);

						}
						else if (tok.checkToken("TexUnit"))
						{
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							sampler.texUnit = (int)atoi(tok.getToken(intnum));
							if (sampler.texUnit > 11)
							{
								LEAN_DEGUG_LOG("error Fx TexUnit不能大于11 在第%d行！", tok.getLine());
								return false;
							}
							if (sampler.texUnit >= 0)
							{
								unitFree[sampler.texUnit] = false;
							}
						}
						else if (tok.checkToken("Address"))
						{
							sampler.sampState &= ~SS_ADDR_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							if (tok.checkToken("Wrap"))
								sampler.sampState |= SS_ADDR_WRAP;
							else if (tok.checkToken("Clamp"))
								sampler.sampState |= SS_ADDR_CLAMP;
							else
							{
								LEAN_DEGUG_LOG("error Fx 错误的纹理Address 在第%d行！", tok.getLine());
								return false;
							}
						}
						else if (tok.checkToken("Filter"))
						{
							sampler.sampState &= ~SS_FILTER_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							if (tok.checkToken("Trilinear"))
								sampler.sampState |= SS_FILTER_TRILINEAR;
							else if (tok.checkToken("Bilinear"))
								sampler.sampState |= SS_FILTER_BILINEAR;
							else if (tok.checkToken("None"))
								sampler.sampState |= SS_FILTER_POINT;
							else
							{
								LEAN_DEGUG_LOG("error Fx 错误的纹理Filter 在第%d行！", tok.getLine());
								return false;
							}
						}
						else if (tok.checkToken("MaxAnisotropy"))
						{
							sampler.sampState &= ~SS_ANISO_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							uint32 maxAniso = (uint32)atoi(tok.getToken(intnum));
							if (maxAniso <= 1) sampler.sampState |= SS_ANISO1;
							else if (maxAniso <= 2) sampler.sampState |= SS_ANISO2;
							else if (maxAniso <= 4) sampler.sampState |= SS_ANISO4;
							else if (maxAniso <= 8) sampler.sampState |= SS_ANISO8;
							else sampler.sampState |= SS_ANISO16;
						}
						else
						{
							LEAN_DEGUG_LOG("error Fx sampler_state错误标识符 在第%d行！", tok.getLine());
							return false;
						}
						if (!tok.checkToken(";"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 ; 在第%d行！", tok.getLine());
							return false;
						}
					}
				}

				if (!tok.checkToken(";"))
				{
					LEAN_DEGUG_LOG("error Fx 缺少 ; 在第%d行！", tok.getLine());
					return false;
				}
				_samplers.push_back(sampler);
			}
			else if (tok.checkToken("context"))
			{
				ShaderContext context;
				_tmpCode0 = _tmpCode1 = "";
				_tmpCode2 = _tmpCode3 = _tmpCode4 = "";
				context.id = tok.getToken(identifier);
				if (context.id == "")
				{
					LEAN_DEGUG_LOG("error Fx 未定义标识符 在第%d行！", tok.getLine());
					return false;
				}
				if (tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx 缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (!tok.checkToken("{"))
				{
					LEAN_DEGUG_LOG("error Fx 缺少 { 在第%d行！", tok.getLine());
					return false;
				}
				while (true)
				{
					if (!tok.hasToken())
					{
						LEAN_DEGUG_LOG("error Fx 缺少 } 在第%d行！", tok.getLine());
						return false;
					}
					else if (tok.checkToken("}"))
					{
						break;
					}
					else if (tok.checkToken("ZWriteEnable"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("true"))
							context.writeDepth = true;
						else if (tok.checkToken("false"))
							context.writeDepth = false;
						else
						{
							LEAN_DEGUG_LOG("error Fx 必须为bool值 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("ZEnable"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("true"))
							context.depthTest = true;
						else if (tok.checkToken("false"))
							context.depthTest = false;
						else
						{
							LEAN_DEGUG_LOG("error Fx 必须为bool值 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("ZFunc"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("LessEqual"))
							context.depthFunc = TestModes::LessEqual;
						else if (tok.checkToken("Always"))
							context.depthFunc = TestModes::Always;
						else if (tok.checkToken("Equal"))
							context.depthFunc = TestModes::Equal;
						else if (tok.checkToken("Less"))
							context.depthFunc = TestModes::Less;
						else if (tok.checkToken("Greater"))
							context.depthFunc = TestModes::Greater;
						else if (tok.checkToken("GreaterEqual"))
							context.depthFunc = TestModes::GreaterEqual;
						else
						{
							LEAN_DEGUG_LOG("error Fx 未定义深度测试枚举值 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("BlendMode"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("Replace")) 
							context.blendMode = BlendModes::Replace;
						else if (tok.checkToken("Blend")) 
							context.blendMode = BlendModes::Blend;
						else if (tok.checkToken("Add")) 
							context.blendMode = BlendModes::Add;
						else if (tok.checkToken("AddBlended")) 
							context.blendMode = BlendModes::AddBlended;
						else if (tok.checkToken("Mult")) 
							context.blendMode = BlendModes::Mult;
						else
						{
							LEAN_DEGUG_LOG("error Fx 未定义混合模式枚举 在第%d行！", tok.getLine());
							return false;
							
						}
					}
					else if (tok.checkToken("CullMode"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("Back")) 
							context.cullMode = CullModes::Back;
						else if (tok.checkToken("Front")) 
							context.cullMode = CullModes::Front;
						else if (tok.checkToken("None")) 
							context.cullMode = CullModes::None;
						else
						{
							LEAN_DEGUG_LOG("error Fx 未定义剔除模式枚举 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("AlphaToCoverage"))
					{
						if (!tok.checkToken("="))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
							return false;
						}
						if (tok.checkToken("true"))
							context.alphaToCoverage = true;
						else if (tok.checkToken("false"))
							context.alphaToCoverage = false;
						else
						{
							LEAN_DEGUG_LOG("error Fx 必须为bool值 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("VertexShader"))
					{
						if (!tok.checkToken("=") || !tok.checkToken("compile") || !tok.checkToken("GLSL"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = compile GLSL 在第%d行！", tok.getLine());
							return false;
						}
						_tmpCode0 = tok.getToken(identifier);
						if (_tmpCode0 == "")
						{
							LEAN_DEGUG_LOG("error Fx 无效vertexshader 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("PixelShader"))
					{
						if (!tok.checkToken("=") || !tok.checkToken("compile") || !tok.checkToken("GLSL"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = compile GLSL 在第%d行！", tok.getLine());
							return false;
						}
						_tmpCode1 = tok.getToken(identifier);
						if (_tmpCode1 == "")
						{
							LEAN_DEGUG_LOG("error Fx 无效fragmentshader 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("GeoShader"))
					{
						if (!tok.checkToken("=") || !tok.checkToken("compile") || !tok.checkToken("GLSL"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = compile GLSL 在第%d行！", tok.getLine());
							return false;
						}
						_tmpCode2 = tok.getToken(identifier);
						if (_tmpCode2 == "")
						{
							LEAN_DEGUG_LOG("error Fx 无效GeoShader 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("TescontShader"))
					{
						if (!tok.checkToken("=") || !tok.checkToken("compile") || !tok.checkToken("GLSL"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = compile GLSL 在第%d行！", tok.getLine());
							return false;
						}
						_tmpCode3 = tok.getToken(identifier);
						if (_tmpCode3 == "")
						{
							LEAN_DEGUG_LOG("error Fx 无效TescontShader 在第%d行！", tok.getLine());
							return false;
						}
					}
					else if (tok.checkToken("TesevalShader"))
					{
						if (!tok.checkToken("=") || !tok.checkToken("compile") || !tok.checkToken("GLSL"))
						{
							LEAN_DEGUG_LOG("error Fx 缺少 = compile GLSL 在第%d行！", tok.getLine());
							return false;
						}
						_tmpCode4 = tok.getToken(identifier);
						if (_tmpCode4 == "")
						{
							LEAN_DEGUG_LOG("error Fx 无效TesevalShader 在第%d行！", tok.getLine());
							return false;
						}
					}
					else
					{
						LEAN_DEGUG_LOG("error Fx 错误标识符 在第%d行！", tok.getLine());
						return false;
					}
					if (!tok.checkToken(";"))
					{
						LEAN_DEGUG_LOG("error Fx 缺少 ; 在第%d行！", tok.getLine());
						return false;
					}
				}
				
				//
				for (uint32 i = 0; i < _codeSections.size(); ++i)
				{
					if (_codeSections[i].getName() == _tmpCode0)
						context.vertCodeIdx = i;
					if (_codeSections[i].getName() == _tmpCode1)
						context.fragCodeIdx = i;
					if (_codeSections[i].getName() == _tmpCode2)
						context.geoCodeIdx = i;
					if (_codeSections[i].getName() == _tmpCode3)
						context.TescontCodeIdx = i;
					if (_codeSections[i].getName() == _tmpCode4)
						context.TesevalCodeIdx = i;
				}
				if (context.vertCodeIdx < 0)
				{
					LEAN_DEGUG_LOG("error context：%s引用的VertexShader未找到！", context.id.c_str());
					return false;
				}
				if (context.fragCodeIdx < 0)
				{
					LEAN_DEGUG_LOG("error context：%s引用的PixelShader未找到！", context.id.c_str());
					return false;
				}
				if (context.geoCodeIdx < 0 && _tmpCode2 != "")
				{
					LEAN_DEGUG_LOG("error context：%s引用的GeometryShader未找到！", context.id.c_str());
					return false;
				}
				if (context.TescontCodeIdx < 0 && _tmpCode3 != "")
				{
					LEAN_DEGUG_LOG("error context：%s引用的TessellationControlShader未找到！", context.id.c_str());
					return false;
				}
				if (context.TesevalCodeIdx < 0 && _tmpCode4 != "")
				{
					LEAN_DEGUG_LOG("error context：%s引用的TessellationEvalutionShader未找到！", context.id.c_str());
					return false;
				}
				_contexts.push_back(context);
			}
			else
			{
				LEAN_DEGUG_LOG("error Fx 错误标识符 在第%d行！", tok.getLine());
				return false;
			}
		}
		//纹理单元分配
		for (uint32 i = 0; i < _samplers.size(); ++i)
		{
			if (_samplers[i].texUnit < 0)
			{
				for (uint32 j = 0; j < 12; ++j)
				{
					if (unitFree[j])
					{
						_samplers[i].texUnit = j;
						unitFree[j] = false;
						break;
					}
				}
				if (_samplers[i].texUnit < 0)
				{
					LEAN_DEGUG_LOG("error Fx 纹理单元不足！", 0);
					return false;
				}
			}
		}

		return true;
	}

	uint32 ShaderResource::calcCombMask(const std::vector<std::string> &flags)
	{
		uint32 combMask = 0;
		for (size_t i = 0; i < flags.size(); ++i)
		{
			const std::string &flag = flags[i];

			if (flag.length() < 5) continue;
			if (flag[0] != '_' || flag[1] != 'F' || flag[4] != '_' 
				|| flag[2] < 48 || flag[2] > 57 || flag[3] < 48 || flag[3] > 57)
				continue;

			uint32 num = (flag[2] - 48) * 10 + (flag[3] - 48);
			combMask |= 1 << (num - 1);
		}

		return combMask;
	}

	void ShaderResource::compileContexts()
	{
		for (uint32 i = 0; i < _contexts.size(); ++i)
		{
			ShaderContext &context = _contexts[i];

			if (!context.compiled)
			{
				context.flagMask = 0;
				if (!getCode(context.vertCodeIdx)->getCodeLinkFlags(&context.flagMask)
					|| !getCode(context.fragCodeIdx)->getCodeLinkFlags(&context.flagMask))
				{
					continue;
				}
				else if ((context.geoCodeIdx != -1 && !getCode(context.geoCodeIdx)->getCodeLinkFlags(&context.flagMask)) 
					|| (context.TescontCodeIdx != -1 && !getCode(context.TescontCodeIdx)->getCodeLinkFlags(&context.flagMask)) 
					|| (context.TesevalCodeIdx != -1 && !getCode(context.TesevalCodeIdx)->getCodeLinkFlags(&context.flagMask)))
				{
					continue;
				}

				//
				for (std::set<uint32>::iterator it = _preLoadList.begin(); it != _preLoadList.end(); ++it)
				{
					uint32 combMask = *it & context.flagMask;
					//检查combination是否已经存在
					bool found = false;
					for (size_t j = 0; j < context.shaderCombs.size(); ++j)
					{
						if (context.shaderCombs[j].combMask == combMask)
						{
							found = true;
							break;
						}
					}
					//如果没有存入过，加入context的comb
					if (!found)
					{
						context.shaderCombs.push_back(ShaderCombination());
						context.shaderCombs.back().combMask = combMask;
					}
				}
				for (size_t j = 0; j < context.shaderCombs.size(); ++j)
				{
					compileCombination(context, context.shaderCombs[j]);
				}
				context.compiled = true;
			}
		}
	}

	void ShaderResource::preLoadCombination(uint32 combMask)
	{
		if (!_loaded)
		{
			_preLoadList.insert(combMask);
		}
		else
		{
			for (uint32 i = 0; i < _contexts.size(); ++i)
			{
				if (getCombination(_contexts[i], combMask) == 0x0)
					_preLoadList.insert(combMask);
			}
		}
	}

	Lean3D::ShaderCombination * ShaderResource::getCombination(ShaderContext &context, uint32 combMask)
	{
		if (!context.compiled)  return 0x0;

		//与操作去掉context中不支持的flag
		combMask &= context.flagMask;

		//
		std::vector<ShaderCombination> &combs = context.shaderCombs;
		for (size_t i = 0; i <  combs.size(); ++i)
		{
			if (combs[i].combMask == combMask) return &combs[i];
		}

		//
		combs.push_back(ShaderCombination());
		combs.back().combMask = combMask;
		compileCombination(context, combs.back());

		return &combs.back();
	}

	void ShaderResource::bindComUniform(uint32 shaderHandle, ShaderCombination &sc)
	{
		// 设置公用uniform
		int loc = g_OGLDiv->shaderManaRef()->getShaderSamplerLoc(shaderHandle, "shadowMap");
		if (loc >= 0) g_OGLDiv->shaderManaRef()->setShaderSampler(loc, 12);

		// 
		sc.uni_frameBufSize = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "frameBufSize");

		// 观察/投影 uniform
		sc.uni_viewMat = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "viewMat");
		sc.uni_viewMatInv = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "viewMatInv");
		sc.uni_projMat = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "projMat");
		sc.uni_viewProjMat = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "viewProjMat");
		sc.uni_viewProjMatInv = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "viewProjMatInv");
		sc.uni_viewerPos = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "viewerPos");

		// 
		sc.uni_worldMat = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "worldMat");
		sc.uni_worldNormalMat = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "worldNormalMat");
		sc.uni_nodeId = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "nodeId");
		sc.uni_customInstData = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "customInstData[0]");
		sc.uni_skinMatRows = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "skinMatRows[0]");

		// 光照计算相关uniform
		sc.uni_lightPos = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "lightPos");
		sc.uni_lightDir = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "lightDir");
		sc.uni_lightColor = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "lightColor");
		sc.uni_shadowSplitDists = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "shadowSplitDists");
		sc.uni_shadowMats = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "shadowMats");
		sc.uni_shadowMapSize = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "shadowMapSize");
		sc.uni_shadowBias = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "shadowBias");

		// 粒子专用 uniforms
		sc.uni_parPosArray = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "parPosArray");
		sc.uni_parSizeAndRotArray = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "parSizeAndRotArray");
		sc.uni_parColorArray = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "parColorArray");

		// Overlay专用 uniforms
		sc.uni_olayColor = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(shaderHandle, "olayColor");
	}

	int ShaderResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case ShaderElemType::ContextElem:
			return (int)_contexts.size();
		case ShaderElemType::SamplerElem:
			return (int)_samplers.size();
		case ShaderElemType::UniformElem:
			return (int)_uniforms.size();
		default:
			return Resource::getElemCount(elem);
		}
	}

	int ShaderResource::getElemParamI(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case ShaderElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case ShaderElemType::UnifSizeI:
					return _uniforms[elemIdx].size;
				}
			}
			break;
		case ShaderElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case ShaderElemType::SampDefTexResI:
					return _samplers[elemIdx].defTex ? _samplers[elemIdx].defTex->getHandle() : 0;
				}
			}
		}

		return Resource::getElemParamI(elem, elemIdx, param);
	}

	float ShaderResource::getElemParamF(int elem, int elemIdx, int param, int compIdx)
	{
		switch (elem)
		{
		case ShaderElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case ShaderElemType::UnifDefValueF4:
					if ((unsigned)compIdx < 4) return _uniforms[elemIdx].defValues[compIdx];
					break;
				}
			}
			break;
		}

		return Resource::getElemParamF(elem, elemIdx, param, compIdx);
	}

	void ShaderResource::setElemParamF(int elem, int elemIdx, int param, int compIdx, float value)
	{
		switch (elem)
		{
		case ShaderElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case ShaderElemType::UnifDefValueF4:
					if ((unsigned)compIdx < 4)
					{
						_uniforms[elemIdx].defValues[compIdx] = value;
						return;
					}
					break;
				}
			}
			break;
		}

		Resource::setElemParamF(elem, elemIdx, param, compIdx, value);
	}

	const char * ShaderResource::getElemParamStr(int elem, int elemIdx, int param)
	{
		switch (elem)
		{
		case ShaderElemType::ContextElem:
			if ((unsigned)elemIdx < _contexts.size())
			{
				switch (param)
				{
				case ShaderElemType::ContNameStr:
					return _contexts[elemIdx].id.c_str();
				}
			}
			break;
		case ShaderElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case ShaderElemType::SampNameStr:
					return _samplers[elemIdx].id.c_str();
				}
			}
			break;
		case ShaderElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case ShaderElemType::UnifNameStr:
					return _uniforms[elemIdx].id.c_str();
				}
			}
			break;
		}

		return Resource::getElemParamStr(elem, elemIdx, param);
	}

	bool ShaderResource::setUniform(const std::string &name, float a, float b, float c, float d)
	{
		//看需要实现
		ASSERT(0);
		return false;
	}

	void ShaderResource::compileCombination(ShaderContext &context, ShaderCombination &sc)
	{
		uint32 combMask = sc.combMask;
		_tmpCode0 = _vertPreamble;
		_tmpCode1 = _fragPreamble;

		if (context.geoCodeIdx != -1)
			_tmpCode2 = _geoPreamble;
		else
			_tmpCode2 = "";
		if (context.TescontCodeIdx != -1)
			_tmpCode3 = _tescontPreamble;
		else
			_tmpCode3 = "";
		if (context.TesevalCodeIdx != -1)
			_tmpCode4 = _tesevalPreamble;
		else
			_tmpCode4 = "";

		if (combMask != 0)
		{
			for (uint32 i = 1; i <= 32; ++i)
			{
				if (combMask & (1 << (i-1)))
				{
					_tmpCode0 += "#define  _F";
					_tmpCode0 += (char)(48+i/10);
					_tmpCode0 += (char)(48+i%10);
					_tmpCode0 += "_\r\n";

					_tmpCode1 += "#define  _F";
					_tmpCode1 += (char)(48 + i / 10);
					_tmpCode1 += (char)(48 + i % 10);
					_tmpCode1 += "_\r\n";

					if (context.geoCodeIdx != -1)
					{
						_tmpCode2 += "#define  _F";
						_tmpCode2 += (char)(48 + i / 10);
						_tmpCode2 += (char)(48 + i % 10);
						_tmpCode2 += "_\r\n";
					}
					if (context.TescontCodeIdx != -1)
					{
						_tmpCode3 += "#define  _F";
						_tmpCode3 += (char)(48 + i / 10);
						_tmpCode3 += (char)(48 + i % 10);
						_tmpCode3 += "_\r\n";
					}
					if (context.TesevalCodeIdx != -1)
					{					
						_tmpCode4 += "#define  _F";
						_tmpCode4 += (char)(48 + i / 10);
						_tmpCode4 += (char)(48 + i % 10);
						_tmpCode4 += "_\r\n";
					}
				}
			}
		}

		_tmpCode0 += getCode(context.vertCodeIdx)->assembleCode();
		_tmpCode1 += getCode(context.fragCodeIdx)->assembleCode();   
		if (context.geoCodeIdx != -1)
		{
			_tmpCode2 += getCode(context.geoCodeIdx)->assembleCode();
		}
		if (context.TescontCodeIdx != -1)
		{
			_tmpCode3 += getCode(context.TescontCodeIdx)->assembleCode();
		}
		if (context.TesevalCodeIdx != -1)
		{
			_tmpCode4 += getCode(context.TesevalCodeIdx)->assembleCode();
		}
		 
		LEAN_DEGUG_LOG("Hit_Log -编译shader---%s", _name.c_str());
		//编译前检查shader组合结构体是否包含曾被编译过的shader
		if (sc.shaderHandle != 0)
		{
			g_OGLDiv->shaderManaRef()->destroyShader(sc.shaderHandle);
			sc.shaderHandle = 0;
		}

		//编译shader
		sc.shaderHandle = g_OGLDiv->shaderManaRef()->createShader(_tmpCode0.c_str()
																, _tmpCode3.c_str()
																, _tmpCode4.c_str()
																, _tmpCode2.c_str()
																, _tmpCode1.c_str());
		//2016/05/23 edit by zhou
		///还未决定是否将公用一致变量在renderer中获取location
		//ASSERT(0);
		//if (1)
		//{
		//}
		//else
		{
			g_OGLDiv->shaderManaRef()->setCurrentShader(sc.shaderHandle);

			bindComUniform(sc.shaderHandle, sc);

			//
			sc.customSamplers.reserve(_samplers.size());
			for (int i = 0; i < _samplers.size(); ++i)
			{
				int loc = g_OGLDiv->shaderManaRef()->getShaderSamplerLoc(sc.shaderHandle
														, _samplers[i].id.c_str());
				sc.customSamplers.push_back(loc);
				
				//设置对应纹理单元
				if (loc >= 0)
					g_OGLDiv->shaderManaRef()->setShaderSampler(loc, _samplers[i].texUnit);
			}

			//
			sc.customUniforms.reserve(_uniforms.size());
			for (int i = 0; i < _uniforms.size(); ++i)
			{
				int loc = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(sc.shaderHandle
														, _uniforms[i].id.c_str());
				sc.customUniforms.push_back(loc);
			}
		}

		g_OGLDiv->shaderManaRef()->setCurrentShader(0);

		///输出shader编译log
		//if ()
	}

}