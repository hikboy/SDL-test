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
	char const * VERT_SHADER_SOURCE("gl-440/texture-bindless.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/texture-bindless.frag");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
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
			INDIRECT,
			TRANSFORM,
			MATERIAL,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_440_texture_bindless_arb : public test
{
public:
	gl_440_texture_bindless_arb(int argc, char* argv[]) :
		test(argc, argv, "gl-440-texture-bindless-arb", test::CORE, 4, 2),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0),
		TextureHandle(0),
		UniformPointer(nullptr)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	GLuint64 TextureHandle;
	glm::mat4* UniformPointer;

	bool initProgram()
	{
		bool Validated(true);
	
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
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, VertexSize, VertexData, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLsizeiptr UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		this->UniformPointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
		glBufferStorage(GL_UNIFORM_BUFFER, sizeof(TextureHandle), &TextureHandle, 0);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		DrawElementsIndirectCommand Command(ElementCount, 1, 0, 0, 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->BufferName[buffer::INDIRECT]);
		glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(Command), &Command, 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D,
			GLint(Texture.levels()),
			gli::internal_format(Texture.format()),
			GLsizei(Texture.dimensions().x), GLsizei(Texture.dimensions().y));

		for(gli::texture2D::size_type Level(0); Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
				Texture[Level].data());
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		// Query the texture handle and make the texture resident
		this->TextureHandle = glGetTextureHandleARB(this->TextureName);
		glMakeTextureHandleResidentARB(this->TextureHandle);

		glBindTexture(GL_TEXTURE_2D, 0);

		return Validated;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_multi_draw_indirect");
		Validated = Validated && this->checkExtension("GL_ARB_bindless_texture");
		Validated = Validated && this->checkExtension("GL_ARB_buffer_storage");
		Validated = Validated && this->checkExtension("GL_ARB_shader_storage_buffer_object");

		this->sync(test::ASYNC);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		if(Validated)
		{
			glm::vec2 WindowSize(this->getWindowSize());

			glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

			glBindProgramPipeline(PipelineName);
			glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
			glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[buffer::INDIRECT]);
			glBindVertexArray(VertexArrayName);
		}

		return Validated;
	}

	bool end()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			*this->UniformPointer = Projection * this->view() * glm::mat4(1.0f);
		}

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), 1, sizeof(DrawElementsIndirectCommand));

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_texture_bindless_arb Test(argc, argv);
	Error += Test();

	return Error;
}

