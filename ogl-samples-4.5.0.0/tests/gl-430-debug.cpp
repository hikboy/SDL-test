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
	char const * VERT_SHADER_SOURCE("gl-430/debug.vert");
	char const * FRAG_SHADER_SOURCE("gl-430/debug.frag");
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

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program

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
}//namespace

class gl_430_debug : public test
{
public:
	gl_430_debug(int argc, char* argv[]) :
		test(argc, argv, "gl-430-debug", test::CORE, 4, 2, glm::vec2(0), test::GENERATE_ERROR),
		PipelineName(0),
		VertexArrayName(0),
		TextureName(0)
	{}

private:
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint VertexArrayName;
	GLuint TextureName;

	bool initProgram()
	{
		bool Validated(true);
	
		glGenProgramPipelines(1, &PipelineName);
		glObjectLabel(GL_PROGRAM_PIPELINE, PipelineName, -1, "Pipeline Program object");

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[program::VERTEX] = glCreateProgram();

			glObjectLabel(GL_PROGRAM, ProgramName[program::VERTEX], -1, "Vertex Program object");

			glProgramParameteri(ProgramName[program::VERTEX], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::VERTEX], VertShaderName);
			glLinkProgram(ProgramName[program::VERTEX]);

			ProgramName[program::FRAGMENT] = glCreateProgram();

			glObjectLabel(GL_PROGRAM, ProgramName[program::FRAGMENT], -1, "Fragment Program object");

			glProgramParameteri(ProgramName[program::FRAGMENT], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::FRAGMENT], FragShaderName);
			glLinkProgram(ProgramName[program::FRAGMENT]);

			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERTEX]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAGMENT]);
		}

		if(Validated)
		{
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERTEX]);
			glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAGMENT]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		bool Validated(true);

		glGenBuffers(buffer::MAX, &BufferName[0]);

		glObjectLabel(GL_BUFFER, BufferName[buffer::ELEMENT], -1, "Element Array Buffer object");

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glObjectLabel(GL_BUFFER, BufferName[buffer::VERTEX], -1, "Array Buffer object");

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glObjectLabel(GL_BUFFER, BufferName[buffer::TRANSFORM], -1, "Uniform Buffer object");
	
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return Validated;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);

		glObjectLabel(GL_TEXTURE, TextureName, -1, "Texture object");

		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Throwing an error on glTexParameteri");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_LINEAR); // Generates an error GL_LINEAR instead of GL_ALPHA 

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glObjectLabel(GL_VERTEX_ARRAY, VertexArrayName, -1, "Vertex array object");

			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return Validated;
	}

	bool initDebug()
	{
		bool Validated(true);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback(&test::debugOutput, this);

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Message test: Begin");

			GLuint MessageId(4);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
			glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 1, &MessageId, GL_FALSE);
			std::string Message1("Message 1");
			glDebugMessageInsertARB(
				GL_DEBUG_SOURCE_APPLICATION_ARB,
				GL_DEBUG_TYPE_OTHER_ARB, 1,
				GL_DEBUG_SEVERITY_MEDIUM_ARB,
				GLsizei(Message1.size()), Message1.c_str());
			std::string Message2("Message 2");
			glDebugMessageInsertARB(
				GL_DEBUG_SOURCE_THIRD_PARTY_ARB,
				GL_DEBUG_TYPE_OTHER_ARB, 2,
				GL_DEBUG_SEVERITY_MEDIUM_ARB,
				GLsizei(Message2.size()), Message2.c_str());
			glDebugMessageInsertARB(
				GL_DEBUG_SOURCE_APPLICATION_ARB,
				GL_DEBUG_TYPE_OTHER_ARB, 2,
				GL_DEBUG_SEVERITY_MEDIUM_ARB,
				-1, "Message 3");
			glDebugMessageInsertARB(
				GL_DEBUG_SOURCE_APPLICATION_ARB,
				GL_DEBUG_TYPE_OTHER_ARB, MessageId,
				GL_DEBUG_SEVERITY_MEDIUM_ARB,
				-1, "Message 4");

		glPopDebugGroup();

		return Validated;
	}

	bool begin()
	{
		bool Validated(true);

		if(Validated && this->checkExtension("GL_KHR_debug"))
			Validated = initDebug();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

		glDebugMessageInsert(
			GL_DEBUG_SOURCE_APPLICATION,
			GL_DEBUG_TYPE_MARKER,
			1,
			GL_DEBUG_SEVERITY_NOTIFICATION,
			-1, 
			"End initialization");

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName[program::FRAGMENT]);
		glDeleteProgram(ProgramName[program::VERTEX]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Frame");

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
	
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, GLfloat(WindowSize.x), GLfloat(WindowSize.y));

		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Throwing an error on glClearBufferfv");
	
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
		//glClearBufferfv(GL_TEXTURE_2D, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]); // Add an error for testing: GL_TEXTURE_2D instead of GL_COLOR

		glBindProgramPipeline(PipelineName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Throwing an error on glDrawElementsInstancedBaseVertexBaseInstance");
		//glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_FLOAT, 0, 1, 0, 0); // Add an error for testing: GL_FLOAT instead of GL_UNSIGNED_SHORT
		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0); // Add an error for testing: GL_FLOAT instead of GL_UNSIGNED_SHORT

		glPopDebugGroup();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_430_debug Test(argc, argv);
	Error += Test();

	return Error;
}
