#include "texture.h"

void Texture::destroy() {
	if (m_id) {
		glDeleteTextures(1, &m_id);
		m_id = 0;
	}
}

Texture& Texture::create(
	TextureType type,
	Format format,
	u32 width, u32 height, u32 depth,
	bool floatingPoint, u32 depthSize
) {
	glGenTextures(1, &m_id);
	m_type = type;
	m_format = format;
	m_floatingPoint = floatingPoint;
	m_depthSize = depthSize;
	m_width = width;
	m_height = height;
	m_depth = depth;
	return *this;
}

Texture& Texture::wrapMode(TextureWrap s, TextureWrap t, TextureWrap r) {
	if (s != TextureWrap::WrapNone) glTexParameteri(m_type, GL_TEXTURE_WRAP_S, s);
	if (t != TextureWrap::WrapNone) glTexParameteri(m_type, GL_TEXTURE_WRAP_T, t);
	if (r != TextureWrap::WrapNone) glTexParameteri(m_type, GL_TEXTURE_WRAP_R, t);
	return *this;
}

Texture& Texture::filter(TextureFilter min, TextureFilter mag) {
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, mag);
	return *this;
}

Texture& Texture::array(u32 layerCount) {
	if (layerCount > 0 && m_type == TextureType::Texture2DArray) {
		glTexStorage3D(
			m_type, 0,
			getInternalFormat(m_format, m_floatingPoint, m_depthSize),
			m_width, m_height,
			layerCount
		);
		m_layerCount = layerCount;
	}
	return *this;
}

Texture& Texture::updateCube(const u8* data, CubeMapSide side, DataType dataType) {
	GLenum ifmt = getInternalFormat(m_format, m_floatingPoint, m_depthSize);
	if (m_type == TextureType::CubeMap) {
		glTexImage2D(side, 0, ifmt, m_width, m_height, 0, m_format, dataType, data);
	}
	return *this;
}

Texture& Texture::updateArray(const u8* data, DataType dataType) {
	if (m_layerCount > 0 && m_type == TextureType::Texture2DArray) {
		glTexSubImage3D(
			m_type,
			0, 0, 0, 0,
			m_width, m_height, m_layerCount,
			m_format,
			dataType,
			data
		);
	} else {
		return update(data, dataType);
	}
	return *this;
}

Texture& Texture::update(const u8* data, DataType dataType) {
	GLenum ifmt = getInternalFormat(m_format, m_floatingPoint, m_depthSize);
	switch (m_type) {
		case TextureType::Texture1D:
			glTexImage1D(m_type, 0, ifmt, m_width, 0, m_format, dataType, data);
			break;
		case TextureType::Texture2D:
			glTexImage2D(m_type, 0, ifmt, m_width, m_height, 0, m_format, dataType, data);
			break;
		case TextureType::Texture3D:
			glTexImage3D(m_type, 0, ifmt, m_width, m_height, m_depth, 0, m_format, dataType, data);
			break;
		default: return *this;
	}
	return *this;
}

Texture& Texture::generateMipmaps() {
	glGenerateMipmap(m_type);
	return *this;
}

Texture& Texture::bind(u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(m_type, m_id);
	return *this;
}

Texture& Texture::unbind() {
	glBindTexture(m_type, 0);
	return *this;
}
