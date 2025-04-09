#include "Program.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

namespace glBurnout {

bool Program::link() {
	if (m_nGLId == 0) {
		if (!initialize()) {
			return false;
		}
	}
	
	glLinkProgram(m_nGLId);
	GLint status;
	glGetProgramiv(m_nGLId, GL_LINK_STATUS, &status);
	return status == GL_TRUE;
}

const std::string Program::getInfoLog() const {
	GLint length;
	glGetProgramiv(m_nGLId, GL_INFO_LOG_LENGTH, &length);
	char* log = new char[length];
	glGetProgramInfoLog(m_nGLId, length, 0, log);
	std::string logString(log);
	delete [] log;
	return logString;
}

// Build a GLSL program from source code
Program buildProgram(const GLchar* vsSrc, const GLchar* fsSrc) {
	Program program;
	program.initialize();  
	
	Shader vs(GL_VERTEX_SHADER);
	vs.setSource(vsSrc);
	
	if(!vs.compile()) {
		std::cerr << "Vertex shader compilation error: " << vs.getInfoLog() << std::endl;
		throw std::runtime_error("Vertex shader compilation failed");
	}
	program.attachShader(vs);
	
	Shader fs(GL_FRAGMENT_SHADER);
	fs.setSource(fsSrc);
	
	if(!fs.compile()) {
		std::cerr << "Fragment shader compilation error: " << fs.getInfoLog() << std::endl;
		throw std::runtime_error("Fragment shader compilation failed");
	}
	program.attachShader(fs);
	
	if(!program.link()) {
		std::cerr << "Program linking error: " << program.getInfoLog() << std::endl;
		throw std::runtime_error("Program linking failed");
	}
	
	glDetachShader(program.getGLId(), vs.getGLId());
	glDetachShader(program.getGLId(), fs.getGLId());
	
	return program;
}

// Load source code from files and build a GLSL program
Program loadProgram(const FilePath& vsFile, const FilePath& fsFile) {
	std::ifstream vsStream(vsFile);
	if(!vsStream) {
		throw std::runtime_error("Cannot open vertex shader file: " + vsFile.str());
	}
	std::stringstream vsSrc;
	vsSrc << vsStream.rdbuf();
	
	std::ifstream fsStream(fsFile);
	if(!fsStream) {
		throw std::runtime_error("Cannot open fragment shader file: " + fsFile.str());
	}
	std::stringstream fsSrc;
	fsSrc << fsStream.rdbuf();
	
	return buildProgram(vsSrc.str().c_str(), fsSrc.str().c_str());
}

}
