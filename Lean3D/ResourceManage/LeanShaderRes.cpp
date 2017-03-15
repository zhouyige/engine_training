#include <fstream>
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
			_include[i].first = "";
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
						resName = LeanRoot::resMana().getRootPath()+ "\\" +resName;
						int bufSize = 1000;
						char *dataBuf = new char[bufSize];
						int fileSize = 0;
						std::ifstream inf;
						inf.open(resName.c_str(), std::ios::binary);
						//打开资源文件
						if (inf.good())
						{
							//获取文件大小
							inf.seekg(0, std::ios::end);
							fileSize = static_cast<int>(inf.tellg());
							if (bufSize < fileSize)
							{
								delete[] dataBuf;
								dataBuf = new char[fileSize + 1];
								if (!dataBuf)
								{
									bufSize = 0;
									continue;
								}
								bufSize = fileSize;
							}
							if (fileSize == 0) continue;
							//复制文件数据输入流到内存
							inf.seekg(0);
							inf.read(dataBuf, fileSize);
							inf.close();
						}
						std::string includeCode = std::string(dataBuf, fileSize);
						_include.push_back(std::pair<std::string, std::string>(resName, includeCode));

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
		updateCode();

		return true;
	}

	void CodeResource::includeCodeReplace(std::string &out, std::string &codename, std::string &code)
	{
		std::string::size_type pos = 0;
		codename = "\"" + codename + "\"";
		std::string::size_type a = codename.size();
		std::string::size_type b = code.size();
		while ((pos = out.find(codename, pos)) != std::string::npos)
		{
			out.replace(pos, a, code);
			pos += b;
		}
	}

	void CodeResource::updateCode()
	{
		std::vector<std::pair<std::string, std::string>>::iterator it = _include.begin();
		for (; it != _include.end(); ++it)
		{
			includeCodeReplace(_code, it->first, it->second);
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
		for (uint32 i = 0; i < _passes.size(); ++i)
		{
			g_OGLDiv->shaderManaRef()->destroyShader(_passes[i].shaderHandle);
		}

		_passes.clear();
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

		compilePasses();

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
			
			if (tok.checkToken("int"))
			{
				ShaderUniform unifrom;
				unifrom._type = ShaderVariableType::INT;
				unifrom._name = tok.getToken(identifier);
				if (unifrom._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
				unifrom._defValue[0] = 0.0f;
				if (tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (tok.checkToken("="))
					unifrom._defValue[0] = (float)atof(tok.getToken(floatnum));
				if (!tok.checkToken(";"))
				{
					LEAN_DEGUG_LOG("error Fx缺少 ; 在第%d行！", tok.getLine());
					return false;
				}
				_uniforms.push_back(unifrom);
			}
			else if(tok.checkToken("float"))
			{
				ShaderUniform unifrom;
				unifrom._type = ShaderVariableType::FLOAT;
				unifrom._name = tok.getToken(identifier);
				if (unifrom._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
				unifrom._defValue[0] = 0.0f;
				if (tok.checkToken("<"))
					if (!tok.seekToken(">"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 > 在第%d行！", tok.getLine());
						return false;
					}
				if (tok.checkToken("="))
					unifrom._defValue[0] = (float)atof(tok.getToken(floatnum));
				if (!tok.checkToken(";"))
				{
					LEAN_DEGUG_LOG("error Fx缺少 ; 在第%d行！", tok.getLine());
					return false;
				}
				_uniforms.push_back(unifrom);
			}
			else if (tok.checkToken("float3"))
			{
				ShaderUniform uniform;
				uniform._type = ShaderVariableType::FLOAT3;
				uniform._name = tok.getToken(identifier);
				if (uniform._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
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
					uniform._defValue[0] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[1] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[2] = (float)atof(tok.getToken(floatnum));
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
			else if (tok.checkToken("float4"))
			{
				ShaderUniform uniform;
				uniform._type = ShaderVariableType::FLOAT4;
				uniform._name = tok.getToken(identifier);
				if (uniform._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
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
					uniform._defValue[0] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[1] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[2] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[3] = (float)atof(tok.getToken(floatnum));
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
			else if (tok.checkToken("float3x3"))
			{
				ShaderUniform uniform;
				uniform._type = ShaderVariableType::FLOAT3x3;
				uniform._name = tok.getToken(identifier);
				if (uniform._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
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
					uniform._defValue[0] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[1] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[2] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[3] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[4] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[5] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[6] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[7] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[8] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[9] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[10] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[11] = (float)atof(tok.getToken(floatnum));
					if (!tok.checkToken("}"))
					{
						LEAN_DEGUG_LOG("error Fx缺少 } 在第%d行！", tok.getLine());
						return false;
					}
				}
			}
			else if (tok.checkToken("float4x4"))
			{
				ShaderUniform uniform;
				uniform._type = ShaderVariableType::FLOAT4x4;
				uniform._name = tok.getToken(identifier);
				if (uniform._name == "")
				{
					LEAN_DEGUG_LOG("error Fx标识符错误！在第%d行！", tok.getLine());
					return false;
				}
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
					uniform._defValue[0] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[1] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[2] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[3] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[4] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[5] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[6] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[7] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[8] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[9] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[10] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[11] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[12] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[13] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[14] = (float)atof(tok.getToken(floatnum));
					if (tok.checkToken(","))
						uniform._defValue[15] = (float)atof(tok.getToken(floatnum));
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
				sampler._sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;
				if (tok.checkToken("sampler2D"))
				{
					sampler._type = TextureType::Tex2D;
					sampler._defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$Tex2D");
				}
				else if (tok.checkToken("sampler3D"))
				{
					sampler._type = TextureType::Tex3D;
					sampler._defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$Tex3D");
				}
				else if (tok.checkToken("samplerCube"))
				{
					sampler._type = TextureType::TexCube;
					sampler._defTex = (TextureResource*)LeanRoot::resMana().findResource(ResourceTypes::Texture, "$TexCube");
				}
				sampler._name = tok.getToken(identifier);
				if (sampler._name == "")
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
							sampler._defTex = (TextureResource *)LeanRoot::resMana().resolveResHandle(texMap);

						}
						else if (tok.checkToken("TexUnit"))
						{
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							sampler._texUnit = (int)atoi(tok.getToken(intnum));
							if (sampler._texUnit > 11)
							{
								LEAN_DEGUG_LOG("error Fx TexUnit不能大于11 在第%d行！", tok.getLine());
								return false;
							}
							if (sampler._texUnit >= 0)
							{
								unitFree[sampler._texUnit] = false;
							}
						}
						else if (tok.checkToken("Address"))
						{
							sampler._sampState &= ~SS_ADDR_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							if (tok.checkToken("Wrap"))
								sampler._sampState |= SS_ADDR_WRAP;
							else if (tok.checkToken("Clamp"))
								sampler._sampState |= SS_ADDR_CLAMP;
							else
							{
								LEAN_DEGUG_LOG("error Fx 错误的纹理Address 在第%d行！", tok.getLine());
								return false;
							}
						}
						else if (tok.checkToken("Filter"))
						{
							sampler._sampState &= ~SS_FILTER_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							if (tok.checkToken("Trilinear"))
								sampler._sampState |= SS_FILTER_TRILINEAR;
							else if (tok.checkToken("Bilinear"))
								sampler._sampState |= SS_FILTER_BILINEAR;
							else if (tok.checkToken("None"))
								sampler._sampState |= SS_FILTER_POINT;
							else
							{
								LEAN_DEGUG_LOG("error Fx 错误的纹理Filter 在第%d行！", tok.getLine());
								return false;
							}
						}
						else if (tok.checkToken("MaxAnisotropy"))
						{
							sampler._sampState &= ~SS_ANISO_MASK;
							if (!tok.checkToken("="))
							{
								LEAN_DEGUG_LOG("error Fx 缺少 = 在第%d行！", tok.getLine());
								return false;
							}
							uint32 maxAniso = (uint32)atoi(tok.getToken(intnum));
							if (maxAniso <= 1) sampler._sampState |= SS_ANISO1;
							else if (maxAniso <= 2) sampler._sampState |= SS_ANISO2;
							else if (maxAniso <= 4) sampler._sampState |= SS_ANISO4;
							else if (maxAniso <= 8) sampler._sampState |= SS_ANISO8;
							else sampler._sampState |= SS_ANISO16;
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
			else if (tok.checkToken("pass"))
			{
				ShaderPass context;
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
				_passes.push_back(context);
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
			if (_samplers[i]._texUnit < 0)
			{
				for (uint32 j = 0; j < 12; ++j)
				{
					if (unitFree[j])
					{
						_samplers[i]._texUnit = j;
						unitFree[j] = false;
						break;
					}
				}
				if (_samplers[i]._texUnit < 0)
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

	void ShaderResource::setPrefixMacros(std::vector<std::string> &macros)
	{
		for (int i = 0; i < macros.size(); ++i)
		{
			_macrosPrefix += "#define "+macros[i]+"\n";
		}
	}

	void ShaderResource::compilePasses()
	{
		for (uint32 i = 0; i < _passes.size(); ++i)
		{
			ShaderPass &pass = _passes[i];

			if (!pass.compiled)
			{
				pass.flagMask = 0;
				compilePassCode(pass);
				pass.compiled = true;
			}
		}
	}

	int ShaderResource::getElemCount(int elem)
	{
		switch (elem)
		{
		case ShaderElemType::ContextElem:
			return (int)_passes.size();
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
					ASSERT(false);
					return 0;
				}
			}
			break;
		case ShaderElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case ShaderElemType::SampDefTexResI:
					return _samplers[elemIdx]._defTex ? _samplers[elemIdx]._defTex->getHandle() : 0;
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
					if ((unsigned)compIdx < 4) return _uniforms[elemIdx]._defValue[compIdx];
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
						_uniforms[elemIdx]._defValue[compIdx] = value;
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
			if ((unsigned)elemIdx < _passes.size())
			{
				switch (param)
				{
				case ShaderElemType::ContNameStr:
					return _passes[elemIdx].id.c_str();
				}
			}
			break;
		case ShaderElemType::SamplerElem:
			if ((unsigned)elemIdx < _samplers.size())
			{
				switch (param)
				{
				case ShaderElemType::SampNameStr:
					return _samplers[elemIdx]._name.c_str();
				}
			}
			break;
		case ShaderElemType::UniformElem:
			if ((unsigned)elemIdx < _uniforms.size())
			{
				switch (param)
				{
				case ShaderElemType::UnifNameStr:
					return _uniforms[elemIdx]._name.c_str();
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

	Lean3D::ShaderPass * ShaderResource::findPass(const std::string &name)
	{
		for (uint32 i = 0; i < _passes.size(); ++i)
			if (_passes[i].id == name) return &_passes[i];

		return 0x0;
	}

	void ShaderResource::compilePassCode(ShaderPass &pass)
	{
		//version and macros 
		_tmpCode0 = _vertPreamble + "\n\n"+ _macrosPrefix;
		_tmpCode1 = _fragPreamble + "\n\n" + _macrosPrefix;

		if (pass.geoCodeIdx != -1)
		{
			_tmpCode2 = _geoPreamble+ "\n\n" + _macrosPrefix;
		}
		else
		{
			_tmpCode2 = "";
		}

		if (pass.TescontCodeIdx != -1)
		{
			_tmpCode3 = _tescontPreamble + "\n\n" + _macrosPrefix;
		}
		else
		{
			_tmpCode3 = "";
		}

		if (pass.TesevalCodeIdx != -1)
		{
			_tmpCode4 = _tesevalPreamble + "\n\n" + _macrosPrefix;
		}
		else
		{
			_tmpCode4 = "";
		}

		_tmpCode0 += getCode(pass.vertCodeIdx)->getCode();
		_tmpCode1 += getCode(pass.fragCodeIdx)->getCode();   
			 
		LEAN_DEGUG_LOG("Hit_Log -编译shader---%s", _name.c_str());
		///TODO:编译前检查shader组合结构体是否包含曾被编译过的shader  

		//编译shader
		pass.shaderHandle = g_OGLDiv->shaderManaRef()->createShader(_tmpCode0.c_str()
																, _tmpCode3.c_str()
																, _tmpCode4.c_str()
																, _tmpCode2.c_str()
																, _tmpCode1.c_str());
		

		
		GLint activeAttribNum = g_OGLDiv->shaderManaRef()->getActiveAttributeNum(pass.shaderHandle);
		if (activeAttribNum > 0)
		{
			int length =  g_OGLDiv->shaderManaRef()->getActiveAttributeMaxLength(pass.shaderHandle);
			if (length > 0)
			{
				char* attribName = new char[length + 1];
				int attribSize = 0;
				GLenum attribType;
				int attribLocation;
				for (int i = 0; i < activeAttribNum; ++i)
				{
					g_OGLDiv->shaderManaRef()->getActiveAttrib(pass.shaderHandle, i, length, NULL, &attribSize, &attribType, attribName);
					attribName[length] = '\0';
					attribLocation = g_OGLDiv->shaderManaRef()->getShaderAttributeLoc(pass.shaderHandle, attribName);

					ShaderAttribute attribute;
					attribute._pass			= &pass;
					attribute._index		= i;
					attribute._name			= attribName;
					attribute._location		= attribLocation;
					attribute.setTypeByGLenum(attribType);
					pass.attributes.emplace_back(attribute);
				}
			}
		}

		GLint activeUniformNum = g_OGLDiv->shaderManaRef()->getActiveUniformNum(pass.shaderHandle);
		if (activeUniformNum > 0)
		{
			int length = g_OGLDiv->shaderManaRef()->getActiveUniformMaxLength(pass.shaderHandle);
			if (length > 0)
			{
				char* uniformName = new char[length + 1];
				int uniformSize = 0;
				GLenum uniformType;
				int uniformLocation;
				for (int i = 0; i < activeUniformNum; ++i)
				{
					g_OGLDiv->shaderManaRef()->getActiveUniform(pass.shaderHandle, i, length, NULL, &uniformSize, &uniformType, uniformName);
					uniformName[length] = '\0';
					if (length > 3)
					{
						// If this is an array uniform, strip array indexers off it since GL does not
						// seem to be consistent across different drivers/implementations in how it returns
						// array uniforms. On some systems it will return "u_matrixArray", while on others
						// it will return "u_matrixArray[0]".
						char* c = strrchr(uniformName, '[');
						if (c)
						{
							*c = '\0';
						}
					}
					uniformLocation = g_OGLDiv->shaderManaRef()->getShaderUniformLoc(pass.shaderHandle, uniformName);
					if (uniformType == GL_SAMPLER_2D || uniformType == GL_SAMPLER_3D || uniformType == GL_SAMPLER_CUBE)
					{
						ShaderSampler sampler;
						sampler._pass				= &pass;
						sampler._index				= i;
						sampler._name				= uniformName;
						sampler._location			= uniformLocation;
						sampler.setTypeByGLenum(uniformType);
						pass.samplers.emplace_back(sampler);
						
					}
					else
					{
						ShaderUniform uniform;
						uniform._pass				= &pass;
						uniform._index				= i;
						uniform._name				= uniformName;
						uniform._location			= uniformLocation;
						uniform.setTypeByGLenum(uniformType);
						pass.uniforms.emplace_back(uniform);
					}
				}
			}
		}
		
		g_OGLDiv->shaderManaRef()->setCurrentShader(0);

		///输出shader编译log
		//if ()
	}

	const char* ShaderUniform::getName() const
	{
		return _name.c_str();
	}

	const Lean3D::ShaderVariableType ShaderUniform::getType() const
	{
		return _type;
	}

	Lean3D::ShaderPass* ShaderUniform::getShaderPass() const
	{
		return _pass;
	}

	void ShaderUniform::setTypeByGLenum(GLenum type)
	{
		switch (type)
		{
		case GL_INT:
		{
			_type = ShaderVariableType::INT;
		}
			break;
		case GL_FLOAT:
		{
			_type = ShaderVariableType::FLOAT;
		}
			break;
		case GL_FLOAT_VEC2:
		{
			_type = ShaderVariableType::FLOAT2;
		}
		break;
		case GL_FLOAT_VEC3:
		{
			_type = ShaderVariableType::FLOAT3;
		}
			break;
		case GL_FLOAT_VEC4:
		{
			_type = ShaderVariableType::FLOAT4;
		}
			break;
		case GL_FLOAT_MAT3:
		{
			_type = ShaderVariableType::FLOAT3x3;
		}
			break;
		case GL_FLOAT_MAT4:
		{
			_type = ShaderVariableType::FLOAT4x4;
		}
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	ShaderUniform::ShaderUniform()
		: _name(""), 
		_location(-1),
		_index(0),
		_pass(NULL),
		_type(ShaderVariableType::NONE)
	{
		for (int i = 0; i < 15; ++i)
		{
			_defValue[i] = 0;
		}
	}

	const char* ShaderAttribute::getName() const
	{
		return _name.c_str();
	}

	const ShaderVariableType ShaderAttribute::getType() const
	{
		return _type;
	}

	Lean3D::ShaderPass* ShaderAttribute::getShaderPass() const
	{
		return _pass;
	}

	void ShaderAttribute::setTypeByGLenum(GLenum type)
	{
		switch (type)
		{
		case GL_INT:
		{
			_type = ShaderVariableType::INT;
		}
		break;
		case GL_FLOAT:
		{
			_type = ShaderVariableType::FLOAT;
		}
		break;
		case GL_FLOAT_VEC2:
		{
			_type = ShaderVariableType::FLOAT2;
		}
		break;
		case GL_FLOAT_VEC3:
		{
			_type = ShaderVariableType::FLOAT3;
		}
		break;
		case GL_FLOAT_VEC4:
		{
			_type = ShaderVariableType::FLOAT4;
		}
		break;
		case GL_FLOAT_MAT3:
		{
			_type = ShaderVariableType::FLOAT3x3;
		}
		break;
		case GL_FLOAT_MAT4:
		{
			_type = ShaderVariableType::FLOAT4x4;
		}
		break;
		default:
			ASSERT(false);
			break;
		}
	}

	ShaderAttribute::ShaderAttribute()
		: _name(""),
		_location(-1),
		_index(0),
		_pass(NULL),
		_type(ShaderVariableType::NONE)
	{
	}

	void ShaderSampler::setTypeByGLenum(GLenum type)
	{
		switch (type)
		{
		case GL_SAMPLER_2D:
			_type = TextureType::List::Tex2D;
			break;
		case GL_SAMPLER_CUBE:
			_type = TextureType::List::TexCube;
			break;
		default:
			ASSERT(false);
			break;
		}
	}

}