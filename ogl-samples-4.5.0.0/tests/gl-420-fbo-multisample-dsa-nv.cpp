///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-420/texture-2d.vert");
	char const * FRAG_SHADER_SOURCE("gl-420/texture-2d.frag");
	char const * TEXTURE_DIFFUSE("kueken3-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 2.0f,-1.5f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-2.0f, 1.5f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			MULTISAMPLE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture

	GLuint VertexArrayName(0);
	GLuint PipelineName(0);
	GLuint ProgramName(0);

	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint TextureName[texture::MAX] = {0, 0, 0};
	GLuint SamplerName(0);

	GLuint FramebufferRenderName(0);
	GLuint FramebufferResolveName(0);
}//namespace

class gl_420_fbo_multisample_dsa_nv : public test
{
public:
	gl_420_fbo_multisample_dsa_nv(int argc, char* argv[]) :
		test(argc, argv, "gl-420-fbo-multisample-dsa-nv", test::CORE, 4, 2, glm::vec2(glm::pi<float>() * 0.2f))
	{}

private:
	bool initProgram()
	{
		bool Validated(true);

		glGenProgramPipelines(1, &PipelineName);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, 
				"--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE,
				"--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, BufferName);
		glNamedBufferDataEXT(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);
		glNamedBufferDataEXT(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
		glNamedBufferDataEXT(BufferName[buffer::TRANSFORM], VertexSize, VertexData, GL_DYNAMIC_DRAW);

		return true;
	}

	bool initSampler()
	{
		glGenSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		glBindSampler(0, SamplerName);

		return true;
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, TextureName);

		glTextureImage2DMultisampleNV(TextureName[texture::MULTISAMPLE], GL_TEXTURE_2D_MULTISAMPLE, 
			4, GL_RGBA, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_TRUE);

		glTextureImage2DEXT(TextureName[texture::COLORBUFFER], GL_TEXTURE_2D, 
			0, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteriEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureImage2DEXT(
				TextureName[texture::DIFFUSE],
				GL_TEXTURE_2D,
				GLint(Level),
				GL_RGB8,
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0, 
				GL_BGR,
				GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}

		if(Texture.levels() == 1)
			glGenerateTextureMipmapEXT(TextureName[texture::DIFFUSE], GL_TEXTURE_2D);

		return true;
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(1, &FramebufferRenderName);
		glNamedFramebufferTextureEXT(FramebufferRenderName, GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		if(glCheckNamedFramebufferStatusEXT(FramebufferRenderName, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glGenFramebuffers(1, &FramebufferResolveName);
		glNamedFramebufferTextureEXT(FramebufferResolveName, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);
		if(glCheckNamedFramebufferStatusEXT(FramebufferResolveName, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), 0);
		glVertexArrayVertexAttribOffsetEXT(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), sizeof(glm::vec2));
		glEnableVertexArrayAttribEXT(VertexArrayName, semantic::attr::POSITION);
		glEnableVertexArrayAttribEXT(VertexArrayName, semantic::attr::TEXCOORD);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_NV_texture_multisample");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, BufferName);
		glDeleteProgram(ProgramName);
		glDeleteTextures(texture::MAX, TextureName);
		glDeleteFramebuffers(1, &FramebufferRenderName);
		glDeleteFramebuffers(1, &FramebufferResolveName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);
		glDeleteProgramPipelines(1, &PipelineName);

		return true;
	}

	void renderFBO()
	{
		{
			glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f,-1.0f, 1.0f));
			glm::mat4 MVP = Perspective * this->view() * Model;

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(MVP), GL_MAP_WRITE_BIT);
			*Pointer = MVP;
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, float(FRAMEBUFFER_SIZE.x), float(FRAMEBUFFER_SIZE.y));
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferRenderName);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
	}

	void renderFB()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Perspective * this->view() * Model;

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(MVP), GL_MAP_WRITE_BIT);
			*Pointer = MVP;
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize.x, WindowSize.y)[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindSampler(0, SamplerName);
		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
	}

	bool render()
	{
		// Clear the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);

		// Step 1: Render the scene in a multisampled framebuffer
		glEnable(GL_MULTISAMPLE);
		renderFBO();
		glDisable(GL_MULTISAMPLE);

		// Step 2: Resolved multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferRenderName);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferResolveName);
		glBlitFramebuffer(
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Step 3: Generated mipmaps
		glGenerateTextureMipmapEXT(TextureName[texture::COLORBUFFER], GL_TEXTURE_2D);

		// Step 4: Render the colorbuffer from the multisampled framebuffer
		renderFB();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_fbo_multisample_dsa_nv Test(argc, argv);
	Error += Test();

	return Error;
}

