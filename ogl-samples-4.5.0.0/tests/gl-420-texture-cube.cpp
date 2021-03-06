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
	char const * VERT_SHADER_SOURCE("gl-420/texture-cube.vert");
	char const * FRAG_SHADER_SOURCE("gl-420/texture-cube.frag");
	char const * TEXTURE_DIFFUSE("cube.dds");//compressed

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const VertexData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f) * 4.0f,
		glm::vec2( 1.0f,-1.0f) * 4.0f,
		glm::vec2( 1.0f, 1.0f) * 4.0f,
		glm::vec2( 1.0f, 1.0f) * 4.0f,
		glm::vec2(-1.0f, 1.0f) * 4.0f,
		glm::vec2(-1.0f,-1.0f) * 4.0f
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

	struct transform
	{
		glm::mat4 MVP;
		glm::mat4 MV;
		glm::vec3 Camera;
	};
}//namespace

class gl_420_texture_cube : public test
{
public:
	gl_420_texture_cube(int argc, char* argv[]) :
		test(argc, argv, "gl-420-texture-cube", test::CORE, 4, 2, glm::vec2(0.0f, -glm::pi<float>() * 0.45f)),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0),
		SamplerName(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	GLuint SamplerName;

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
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);

		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);

		GLint UniformBlockSize = glm::max(GLint(sizeof(transform)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initSampler()
	{
		glGenSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
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
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

		return true;
	}

	bool initTexture()
	{
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &TextureName);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, TextureName);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, 2);

		gli::textureCube TextureA(6, 1, gli::RGBA8_UNORM, gli::textureCube::dimensions_type(512));
		assert(!TextureA.empty());
		TextureA[0].clear<glm::u8vec4>(glm::u8vec4(255,   0,   0, 255));
		TextureA[1].clear<glm::u8vec4>(glm::u8vec4(255, 128,   0, 255));
		TextureA[2].clear<glm::u8vec4>(glm::u8vec4(255, 255,   0, 255));
		TextureA[3].clear<glm::u8vec4>(glm::u8vec4(  0, 255,   0, 255));
		TextureA[4].clear<glm::u8vec4>(glm::u8vec4(  0, 255, 255, 255));
		TextureA[5].clear<glm::u8vec4>(glm::u8vec4(  0,   0, 255, 255));

		gli::textureCube TextureB(6, 1, gli::RGBA8_UNORM, gli::textureCube::dimensions_type(256));
		assert(!TextureB.empty());
		TextureB[0].clear<glm::u8vec4>(glm::u8vec4(255, 128, 128, 255));
		TextureB[1].clear<glm::u8vec4>(glm::u8vec4(255, 192, 128, 255));
		TextureB[2].clear<glm::u8vec4>(glm::u8vec4(255, 255, 128, 255));
		TextureB[3].clear<glm::u8vec4>(glm::u8vec4(128, 255, 128, 255));
		TextureB[4].clear<glm::u8vec4>(glm::u8vec4(128, 255, 255, 255));
		TextureB[5].clear<glm::u8vec4>(glm::u8vec4(128, 128, 255, 255));

		gli::textureCube TextureC(6, 1, gli::RGBA8_UNORM, gli::textureCube::dimensions_type(128));
		assert(!TextureC.empty());
		TextureC[0].clear<glm::u8vec4>(glm::u8vec4(255, 192, 192, 255));
		TextureC[1].clear<glm::u8vec4>(glm::u8vec4(255, 224, 192, 255));
		TextureC[2].clear<glm::u8vec4>(glm::u8vec4(255, 255, 192, 255));
		TextureC[3].clear<glm::u8vec4>(glm::u8vec4(192, 255, 192, 255));
		TextureC[4].clear<glm::u8vec4>(glm::u8vec4(192, 255, 255, 255));
		TextureC[5].clear<glm::u8vec4>(glm::u8vec4(192, 192, 255, 255));

		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 3,
			GL_RGBA8, GLsizei(TextureA.dimensions().x), GLsizei(TextureA.dimensions().y), GLsizei(TextureA.faces()));

		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0,
			0, 0, 0,
			static_cast<GLsizei>(TextureA.dimensions().x),
			static_cast<GLsizei>(TextureA.dimensions().y),
			static_cast<GLsizei>(TextureA.faces()),
			GL_RGBA, GL_UNSIGNED_BYTE,
			TextureA.data());

		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1,
			0, 0, 0,
			static_cast<GLsizei>(TextureB.dimensions().x),
			static_cast<GLsizei>(TextureB.dimensions().y),
			static_cast<GLsizei>(TextureB.faces()),
			GL_RGBA, GL_UNSIGNED_BYTE,
			TextureB.data());

		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 2,
			0, 0, 0,
			static_cast<GLsizei>(TextureC.dimensions().x),
			static_cast<GLsizei>(TextureC.dimensions().y),
			static_cast<GLsizei>(TextureC.faces()),
			GL_RGBA, GL_UNSIGNED_BYTE,
			TextureC.data());

		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initSampler();

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteTextures(1, &TextureName);
		glDeleteSamplers(1, &SamplerName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			transform* Pointer = (transform*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0, sizeof(transform),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 1000.0f);
			glm::mat4 View = this->view();
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Projection * View * Model;
			glm::mat4 MV = View * Model;

			Pointer->MVP = Projection * View * Model;
			Pointer->MV = View * Model;
			Pointer->Camera = glm::vec3(0.0f, 0.0f, -this->cameraDistance());

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, TextureName);
		glBindSampler(0, SamplerName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glBindVertexArray(VertexArrayName);
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_texture_cube Test(argc, argv);
	Error += Test();

	return Error;
}
