#include "shader.h"

#include <vector>
#include <iostream>

Shader::Shader() {
}

Shader::~Shader() {
	if (m_id) {
		unbind();
		glDeleteProgram(m_id);
		std::cout << "DEL SHADER" << std::endl;
	}
}

Shader& Shader::create() {
	m_id = glCreateProgram();
	return *this;
}

Shader& Shader::bind() {
	glUseProgram(m_id);
	return *this;
}

Shader& Shader::unbind() {
	glUseProgram(0);
	return *this;
}

Shader& Shader::add(const std::string& source, Shader::ShaderType type) {
	if (m_subShaders.find(type) != m_subShaders.end()) {
		return *this;
	}

	GLuint s = glCreateShader(GLenum(type));

	const char* src = source.c_str();
	glShaderSource(s, 1, &src, nullptr);
	glCompileShader(s);

	GLint isCompiled = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		i32 maxLength = 0;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<char> infoLog(maxLength);
		glGetShaderInfoLog(s, maxLength, &maxLength, &infoLog[0]);

		glDeleteShader(s);

		return *this;
	}

	glAttachShader(m_id, s);
	m_subShaders[type] = s;

	return *this;
}

Shader& Shader::link() {
	glLinkProgram(m_id);

	for (auto&& [type, shader] : m_subShaders) {
		glDetachShader(m_id, shader);
		glDeleteShader(shader);
	}

	return *this;
}

i32 Shader::getBlockIndex(Shader::ProgramInterface interface, const std::string& name) {
	auto pos = m_blockIndices.find(name);
	if (pos == m_blockIndices.end()) {
		i32 i = -2;
		switch (interface) {
			case Shader::ShaderStorageBlock:
				i = glGetProgramResourceIndex(m_id, GL_SHADER_STORAGE_BLOCK, name.c_str());
				break;
			case Shader::UniformBufferBlock:
				i = glGetUniformBlockIndex(m_id, name.c_str());
				break;
		}
		if (i != -1 && i != -2) {
			m_blockIndices[name] = i;
		} else {
			return -1;
		}
	}
	return m_blockIndices[name];
}

i32 Shader::getUniformIndex(const std::string& name) {
	auto pos = m_uniforms.find(name);
	if (pos == m_uniforms.end()) {
		i32 i = glGetProgramResourceLocation(m_id, GL_UNIFORM, name.c_str());
		if (i != -1) {
			m_uniforms[name] = i;
		} else {
			return -1;
		}
	}
	return m_uniforms[name];
}

i32 Shader::getAttributeIndex(const std::string& name) {
	auto pos = m_attributes.find(name);
	if (pos == m_attributes.end()) {
		i32 i = glGetAttribLocation(m_id, name.c_str());
		if (i != -1) {
			m_attributes[name] = i;
		} else {
			return -1;
		}
	}
	return m_attributes[name];
}

void Shader::uniformBlockBinding(u32 blockIndex, u32 binding) {
	glUniformBlockBinding(m_id, blockIndex, binding);
}

void Shader::storageBlockBinding(u32 blockIndex, u32 binding) {
	glShaderStorageBlockBinding(m_id, blockIndex, binding);
}

void Shader::Uniform::set(i32 v) {
	glUniform1i(loc, v);
}

void Shader::Uniform::set(f32 v) {
	glUniform1f(loc, v);
}

void Shader::Uniform::set(f32 x, f32 y) {
	glUniform2f(loc, x, y);
}

void Shader::Uniform::set(f32 x, f32 y, f32 z) {
	glUniform3f(loc, x, y, z);
}

void Shader::Uniform::set(f32 x, f32 y, f32 z, f32 w) {
	glUniform4f(loc, x, y, z, w);
}

void Shader::Uniform::mat3(const f32* v, u32 count, bool transpose) {
	glUniformMatrix3fv(loc, count, transpose, v);
}

void Shader::Uniform::mat4(const f32* v, u32 count, bool transpose) {
	glUniformMatrix4fv(loc, count, transpose, v);
}

Shader::Uniform Shader::get(const std::string& name) {
	Uniform uni;
	uni.loc = getUniformIndex(name);
	return uni;
}