#include "buffer.h"

#include <iostream>

void Buffer::destroy() {
	if (m_id) {
		glDeleteBuffers(1, &m_id);
		m_id = 0;
	}
}

Buffer& Buffer::create(BufferType type) {
	m_type = type;
	glGenBuffers(1, &m_id);
	return *this;
}

Buffer& Buffer::bind() {
	glBindBuffer(GLenum(m_type), m_id);
	return *this;
}

Buffer& Buffer::unbind() {
	glBindBuffer(GLenum(m_type), 0);
	return *this;
}

Buffer& Buffer::bindBase(u32 bindingPoint) {
	glBindBufferBase(GLenum(m_type), bindingPoint, m_id);
	return *this;
}

void Buffer::unmap() {
	glUnmapBuffer(GLenum(m_type));
}

VertexArray& VertexArray::create() {
	glGenVertexArrays(1, &m_id);
	return *this;
}

VertexArray& VertexArray::bind() {
	glBindVertexArray(m_id);
	return *this;
}

VertexArray& VertexArray::unbind() {
	glBindVertexArray(0);
	return *this;
}