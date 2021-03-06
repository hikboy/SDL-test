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
	char const * VERT_SHADER_SOURCE("gl-450/query-statistics.vert");
	char const * FRAG_SHADER_SOURCE("gl-450/query-statistics.frag");
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

	namespace statistics
	{
		enum type
		{
			VERTICES_SUBMITTED,
			PRIMITIVES_SUBMITTED,
			VERTEX_SHADER_INVOCATIONS,
			TESS_CONTROL_SHADER_PATCHES,
			TESS_EVALUATION_SHADER_INVOCATIONS,
			GEOMETRY_SHADER_INVOCATIONS,
			GEOMETRY_SHADER_PRIMITIVES_EMITTED,
			FRAGMENT_SHADER_INVOCATIONS,
			COMPUTE_SHADER_INVOCATIONS,
			CLIPPING_INPUT_PRIMITIVES,
			CLIPPING_OUTPUT_PRIMITIVES,
			MAX
		};
	}//namespace statistics
}//namespace

class gl_450_query_statistics : public test
{
public:
	gl_450_query_statistics(int argc, char* argv[]) :
		test(argc, argv, "gl-450-query-statistics-arb", test::CORE, 4, 2),
		PipelineName(0),
		VertexArrayName(0),
		TextureName(0)
	{}

private:
	GLuint PipelineName;
	GLuint VertexArrayName;
	GLuint TextureName;
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, statistics::MAX> QueryName;

	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[program::VERTEX] = glCreateProgram();
			glProgramParameteri(ProgramName[program::VERTEX], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::VERTEX], VertShaderName);
			glLinkProgram(ProgramName[program::VERTEX]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERTEX]);

			ProgramName[program::FRAGMENT] = glCreateProgram();
			glProgramParameteri(ProgramName[program::FRAGMENT], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::FRAGMENT], FragShaderName);
			glLinkProgram(ProgramName[program::FRAGMENT]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAGMENT]);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERTEX]);
			glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAGMENT]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		bool Validated(true);

		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureName);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
			GLint(Texture.levels()),
			gli::internal_format(Texture.format()),
			GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y), GLsizei(1));

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				GLint(Level),
				0, 0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLsizei(1),
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
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

	bool initQuery()
	{
		glGenQueries(statistics::MAX, &QueryName[0]);

		std::array<GLint, statistics::MAX> QueryCounterBits;

		glGetQueryiv(GL_VERTICES_SUBMITTED_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::VERTICES_SUBMITTED]);
		glGetQueryiv(GL_PRIMITIVES_SUBMITTED_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::PRIMITIVES_SUBMITTED]);
		glGetQueryiv(GL_VERTEX_SHADER_INVOCATIONS_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::VERTEX_SHADER_INVOCATIONS]);
		glGetQueryiv(GL_TESS_CONTROL_SHADER_PATCHES_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::TESS_CONTROL_SHADER_PATCHES]);
		glGetQueryiv(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::TESS_EVALUATION_SHADER_INVOCATIONS]);
		glGetQueryiv(GL_GEOMETRY_SHADER_INVOCATIONS, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::GEOMETRY_SHADER_INVOCATIONS]);
		glGetQueryiv(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::GEOMETRY_SHADER_PRIMITIVES_EMITTED]);
		glGetQueryiv(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::FRAGMENT_SHADER_INVOCATIONS]);
		glGetQueryiv(GL_COMPUTE_SHADER_INVOCATIONS_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::COMPUTE_SHADER_INVOCATIONS]);
		glGetQueryiv(GL_CLIPPING_INPUT_PRIMITIVES_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::CLIPPING_INPUT_PRIMITIVES]);
		glGetQueryiv(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, GL_QUERY_COUNTER_BITS, &QueryCounterBits[statistics::CLIPPING_OUTPUT_PRIMITIVES]);

		bool Validated = true;
		for(std::size_t i = 0; i < QueryCounterBits.size(); ++i)
			Validated = Validated && QueryCounterBits[i] >= 18;

		return Validated;
	}

	bool begin()
	{
		bool Validated = this->checkExtension("GL_ARB_pipeline_statistics_query");

		if(Validated)
			Validated = initQuery();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

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

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glBeginQuery(GL_VERTICES_SUBMITTED_ARB, this->QueryName[statistics::VERTICES_SUBMITTED]);
		glBeginQuery(GL_PRIMITIVES_SUBMITTED_ARB, this->QueryName[statistics::PRIMITIVES_SUBMITTED]);
		glBeginQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB, this->QueryName[statistics::VERTEX_SHADER_INVOCATIONS]);
		glBeginQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB, this->QueryName[statistics::TESS_CONTROL_SHADER_PATCHES]);
		glBeginQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, this->QueryName[statistics::TESS_EVALUATION_SHADER_INVOCATIONS]);
		glBeginQuery(GL_GEOMETRY_SHADER_INVOCATIONS, this->QueryName[statistics::GEOMETRY_SHADER_INVOCATIONS]);
		glBeginQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, this->QueryName[statistics::GEOMETRY_SHADER_PRIMITIVES_EMITTED]);
		glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, this->QueryName[statistics::FRAGMENT_SHADER_INVOCATIONS]);
		glBeginQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB, this->QueryName[statistics::COMPUTE_SHADER_INVOCATIONS]);
		glBeginQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB, this->QueryName[statistics::CLIPPING_INPUT_PRIMITIVES]);
		glBeginQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, this->QueryName[statistics::CLIPPING_OUTPUT_PRIMITIVES]);
			glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);
		glEndQuery(GL_VERTICES_SUBMITTED_ARB);
		glEndQuery(GL_PRIMITIVES_SUBMITTED_ARB);
		glEndQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB);
		glEndQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB);
		glEndQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB);
		glEndQuery(GL_GEOMETRY_SHADER_INVOCATIONS);
		glEndQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB);
		glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
		glEndQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB);
		glEndQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB);
		glEndQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB);

		std::array<GLuint, statistics::MAX> QueryResult;
		for(std::size_t i = 0; i < QueryResult.size(); ++i)
			glGetQueryObjectuiv(this->QueryName[i], GL_QUERY_RESULT, &QueryResult[i]);

		fprintf(stdout, "Verts: %d; Prims: (%d, %d); Shaders(%d, %d, %d, %d, %d, %d); Clip(%d, %d)\r",
			QueryResult[statistics::VERTICES_SUBMITTED],
			QueryResult[statistics::PRIMITIVES_SUBMITTED],
			QueryResult[statistics::GEOMETRY_SHADER_PRIMITIVES_EMITTED],
			QueryResult[statistics::VERTEX_SHADER_INVOCATIONS],
			QueryResult[statistics::TESS_CONTROL_SHADER_PATCHES],
			QueryResult[statistics::TESS_EVALUATION_SHADER_INVOCATIONS],
			QueryResult[statistics::GEOMETRY_SHADER_INVOCATIONS],
			QueryResult[statistics::FRAGMENT_SHADER_INVOCATIONS],
			QueryResult[statistics::COMPUTE_SHADER_INVOCATIONS],
			QueryResult[statistics::CLIPPING_INPUT_PRIMITIVES],
			QueryResult[statistics::CLIPPING_OUTPUT_PRIMITIVES]);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_450_query_statistics Test(argc, argv);
	Error += Test();

	return Error;
}
