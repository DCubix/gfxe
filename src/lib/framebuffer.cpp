#include "framebuffer.h"

FrameBuffer::~FrameBuffer() {
	if (m_id) {
		glDeleteFramebuffers(1, &m_id);
		m_id = 0;
	}
	if (m_rboID) {
		glDeleteRenderbuffers(1, &m_rboID);
		m_rboID = 0;
	}
}

FrameBuffer& FrameBuffer::create(u32 width, u32 height, u32 depth) {
	m_width = width;
	m_height = height;
	m_depth = depth;

	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::color(
	TextureType type, Format format,
	bool floatingPoint,
	u32 depthSize, u32 mip, u32 layer
) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	DataType dt = floatingPoint ? DataType::TypeFloat : DataType::TypeUByte;
	Texture tex{};
	tex.create(type, format, m_width, m_height, m_depth, floatingPoint, depthSize).bind();
	if (type == TextureType::CubeMap) {
		tex.updateCube(nullptr, CubeMapSide::NegativeX, dt);
		tex.updateCube(nullptr, CubeMapSide::NegativeY, dt);
		tex.updateCube(nullptr, CubeMapSide::NegativeZ, dt);
		tex.updateCube(nullptr, CubeMapSide::PositiveX, dt);
		tex.updateCube(nullptr, CubeMapSide::PositiveY, dt);
		tex.updateCube(nullptr, CubeMapSide::PositiveZ, dt);
	} else if (type == TextureType::Texture2DArray) {
		tex.updateArray(nullptr, dt);
	} else {
		tex.update(nullptr, dt);
	}
	tex.generateMipmaps();

	SavedColorAttachment sca;
	sca.format = format;
	sca.target = type;
	sca.mip = mip;
	m_savedColorAttachments.push_back(sca);

	std::vector<GLenum> db;
	u32 att = m_colorAttachments.size();
	for (u32 i = 0; i < att + 1; i++) {
		db.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	u32 atc = GL_COLOR_ATTACHMENT0 + att;
	switch (type) {
		case TextureType::Texture1D:
			glFramebufferTexture1D(GL_FRAMEBUFFER, atc, type, tex.id(), mip);
			break;
		case TextureType::Texture2D:
			glFramebufferTexture2D(GL_FRAMEBUFFER, atc, type, tex.id(), mip);
			break;
		case TextureType::Texture3D:
			glFramebufferTexture3D(GL_FRAMEBUFFER, atc, type, tex.id(), mip, layer);
			break;
		case TextureType::Texture2DArray:
			glFramebufferTextureLayer(GL_FRAMEBUFFER, atc, tex.id(), mip, layer);
			break;
	}

	glDrawBuffers(db.size(), db.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return *this;
	}

	m_colorAttachments.push_back(tex);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return *this;
}

FrameBuffer& FrameBuffer::depth(u32 depthSize) {
	if (m_depthAttachment.id() != 0) {
		return *this;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	Texture tex{};
	tex.create(TextureType::Texture2D, Format::Depth, m_width, m_height, 1, true, depthSize);

	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D,
			tex.id(),
			0
	);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_depthAttachment = tex;

	return *this;
}

FrameBuffer& FrameBuffer::stencil() {
	if (m_stencilAttachment.id() != 0) {
		return *this;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	Texture tex{};
	tex.create(TextureType::Texture2D, Format::R, m_width, m_height, 1, true);

	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D,
			tex.id(),
			0
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_stencilAttachment = tex;

	return *this;
}

FrameBuffer& FrameBuffer::renderBuffer(
	Format storage,
	Attachment attachment,
	bool floatingPoint,
	u32 depthSize
) {
	if (m_rboID != 0) {
		return *this;
	}

	m_renderBufferStorage = storage;
	GLenum ifmt = getInternalFormat(storage, floatingPoint, depthSize);
	glGenRenderbuffers(1, &m_rboID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rboID);
	glRenderbufferStorage(GL_RENDERBUFFER, ifmt, m_width, m_height);
	glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			attachment,
			GL_RENDERBUFFER,
			m_rboID
	);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteRenderbuffers(1, &m_rboID);
		m_rboID = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *this;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return *this;
}

void FrameBuffer::drawBuffer(u32 index) {
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + index);
}

void FrameBuffer::resetDrawBuffers() {
	std::vector<GLenum> db;
	i32 att = m_colorAttachments.size();
	for (int i = 0; i < att; i++) {
		db.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers(db.size(), db.data());
}

void FrameBuffer::blit(
	int sx0, int sy0, int sx1, int sy1,
	int dx0, int dy0, int dx1, int dy1,
	ClearBufferMask mask,
	TextureFilter filter)
{
	glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
}

void FrameBuffer::bind(FrameBufferTarget target, Attachment readBuffer) {
	m_bound = target;
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	glBindFramebuffer(target, m_id);
	glViewport(0, 0, m_width, m_height);
	if (target == FrameBufferTarget::ReadFrameBuffer)
		glReadBuffer(readBuffer);
}

void FrameBuffer::unbind(bool resetViewport) {
	glBindFramebuffer(m_bound, 0);
	if (resetViewport) {
		glViewport(
			m_viewport[0],
			m_viewport[1],
			m_viewport[2],
			m_viewport[3]
		);
	}
}