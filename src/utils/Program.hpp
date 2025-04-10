#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Shader.hpp"
#include "FilePath.hpp"

namespace glBurnout {

class Program {
public:
	Program(): m_nGLId(0) {
	}
	
	bool initialize() {
		if (m_nGLId == 0) {
			m_nGLId = glCreateProgram();
			return m_nGLId != 0;
		}
		return true;
	}

	~Program() {
		if (m_nGLId != 0) {
			glDeleteProgram(m_nGLId);
		}
	}

	Program(Program&& rvalue): m_nGLId(rvalue.m_nGLId) {
		rvalue.m_nGLId = 0;
	}

	Program& operator =(Program&& rvalue) {
		m_nGLId = rvalue.m_nGLId;
		rvalue.m_nGLId = 0;
		return *this;
	}

	GLuint getGLId() const {
		return m_nGLId;
	}

	void attachShader(const Shader& shader) {
		if (m_nGLId == 0) {
			initialize();
		}
		glAttachShader(m_nGLId, shader.getGLId());
	}

	bool link();

	const std::string getInfoLog() const;

	void use() const {
		if (m_nGLId != 0) {
			glUseProgram(m_nGLId);
		}
	}

private:
	Program(const Program&);
	Program& operator =(const Program&);

	GLuint m_nGLId;
};

// Build a GLSL program from source code
Program buildProgram(const GLchar* vsSrc, const GLchar* fsSrc);

// Load source code from files and build a GLSL program
Program loadProgram(const FilePath& vsFile, const FilePath& fsFile);

}
