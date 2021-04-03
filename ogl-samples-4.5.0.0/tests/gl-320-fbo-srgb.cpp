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
	char const * VERTEX_SHADER_SOURCE("gl-320/fbo-srgb.vert");
	char const * FRAGMENT_SHADER_SOURCE("gl-320/fbo-srgb.frag");
	char const * TEXTURE_DIFFUSE("kueken3-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(1024, 1024);

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

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture

	std::vector<GLuint> TextureName(texture::MAX);
	GLuint VertexArrayName(0);
	GLuint BufferName(0);
	GLuint FramebufferName(0);
	GLuint ProgramName(0);
	GLint UniformMVP(0);
	GLint UniformDiffuse(0);
	GLint CapableSRGB(0);
}//namespace

class gl_320_fbo_srgb : public test
{
public:
	gl_320_fbo_srgb(int argc, char* argv[]) :
		test(argc, argv, "gl-320-fbo-srgb", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERTEX_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAGMENT_SHADER_SOURCE, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);

			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName, semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
			UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(1, &BufferName);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, &TextureName[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(
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
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(1, &FramebufferName);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return this->checkError("initFramebuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteProgram(ProgramName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteBuffers(1, &BufferName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	void renderScene(glm::vec4 const & ClearColor, glm::mat4 const & MVP, GLuint TextureName)
	{
		glUseProgram(ProgramName);
		glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glUniform1i(UniformDiffuse, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);

		glBindVertexArray(VertexArrayName);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);

		this->checkError("renderScene");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);

		glEnable(GL_SCISSOR_TEST);
		glDisable(GL_FRAMEBUFFER_SRGB);
		glScissor(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		{
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / float(FRAMEBUFFER_SIZE.y), 0.1f, 100.0f);
			glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
			glm::mat4 MVP = Projection * View * Model;

			glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
			glDisable(GL_FRAMEBUFFER_SRGB);

			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);
			renderScene(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), MVP, TextureName[texture::DIFFUSE]);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		{
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, float(WindowSize.x) / float(WindowSize.y), 0.1f, 100.0f);
			glm::mat4 MVP = Projection * View * Model;

			glViewport(0, 0, WindowSize.x, WindowSize.y);

			// Correct display
			glScissor(0, WindowSize.y / 2 - 1, WindowSize.x, WindowSize.y / 2);
			glEnable(GL_FRAMEBUFFER_SRGB);
			renderScene(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), MVP, TextureName[texture::COLORBUFFER]);
			glDisable(GL_FRAMEBUFFER_SRGB);

			// Incorrected display
			glScissor(0, 0, WindowSize.x, WindowSize.y / 2);
			renderScene(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), MVP, TextureName[texture::COLORBUFFER]);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_fbo_srgb Test(argc, argv);
	Error += Test();

	return Error;
}

