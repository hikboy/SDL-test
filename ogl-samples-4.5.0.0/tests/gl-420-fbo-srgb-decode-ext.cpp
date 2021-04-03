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
	char const * VERT_SHADER_SOURCE("gl-420/fbo-srgb-decode.vert");
	char const * FRAG_SHADER_SOURCE("gl-420/fbo-srgb-decode.frag");
	char const * TEXTURE_DIFFUSE("kueken2-dxt1.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(1024);

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	GLuint VertexArrayName(0);
	GLuint PipelineName(0);
	GLuint ProgramName(0);
	GLuint SamplerName(0);

	GLuint BufferName[buffer::MAX] = {0, 0};
	GLuint TextureName = 0;
	GLuint ColorbufferName = 0;
	GLuint FramebufferName = 0;

	GLint UniformMVP = 0;
}//namespace

class gl_420_fbo_srgb_decode_ext : public test
{
public:
	gl_420_fbo_srgb_decode_ext(int argc, char* argv[]) :
		test(argc, argv, "gl-420-fbo-srgb-decode-ext", test::CORE, 4, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated(true);
	
		glGenProgramPipelines(1, &PipelineName);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");

		if(Validated)
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

		return Validated;
	}

	bool initBuffer()
	{
		bool Validated(true);

		glGenBuffers(buffer::MAX, BufferName);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return Validated;
	}

	bool initSampler()
	{
		bool Validated(true);

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
	
		return Validated;
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());
		if(Texture.empty())
			return false;

		glGenTextures(1, &TextureName);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
			GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(
				GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
				GLsizei(Texture[Level].size()), 
				Texture[Level].data());
		}

		return true;
	}

	bool initFramebuffer()
	{
		bool Validated(true);

		glGenTextures(1, &ColorbufferName);
		glBindTexture(GL_TEXTURE_2D, ColorbufferName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ColorbufferName, 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return Validated;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	void renderScene(glm::mat4 const & MVP, GLuint const & TextureName)
	{
		glProgramUniformMatrix4fv(ProgramName, UniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindSampler(0, SamplerName);

		glBindVertexArray(VertexArrayName);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	bool begin()
	{
		bool Validated = true;
		Validated = Validated && this->checkExtension("GL_EXT_texture_sRGB_decode");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteTextures(1, &ColorbufferName);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);

		glDisable(GL_FRAMEBUFFER_SRGB);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
		glBindProgramPipeline(PipelineName);

		{
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / float(FRAMEBUFFER_SIZE.y), 0.1f, 100.0f);
			glm::mat4 MVP = Projection * View * Model;

			glViewportIndexedf(0, 0, 0, float(FRAMEBUFFER_SIZE.x), float(FRAMEBUFFER_SIZE.y));
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

			glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);
			renderScene(MVP, TextureName);
		}

		{
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 MVP = Projection * View * Model;

			glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize.x, WindowSize.y)[0]);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glEnable(GL_SCISSOR_TEST);

			// Correct display
			glScissorIndexed(0, 0, WindowSize.y / 2 - 1, WindowSize.x, WindowSize.y / 2);
			glEnable(GL_FRAMEBUFFER_SRGB);
			glSamplerParameteri(SamplerName, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT); // GL_DECODE_EXT 
			renderScene(MVP, ColorbufferName);
			glDisable(GL_FRAMEBUFFER_SRGB);

			// Incorrected display
			glScissorIndexed(0, 0, 0, WindowSize.x, WindowSize.y / 2);
			glEnable(GL_FRAMEBUFFER_SRGB);
			glSamplerParameteri(SamplerName, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT); // GL_DECODE_EXT 
			renderScene(MVP, ColorbufferName);
			glDisable(GL_FRAMEBUFFER_SRGB);

			glDisable(GL_SCISSOR_TEST);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_fbo_srgb_decode_ext Test(argc, argv);
	Error += Test();

	return Error;
}

