#include "FrameBuffer.h"

#include "Middleware.h"

namespace Server::Engine::Graphics {

	FrameBuffer::FrameBuffer(int width, int height) {
		GLint maximumFrameBufferSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumFrameBufferSize);
		if(width < 0) {
			width = maximumFrameBufferSize;
		}
		if(height < 0) {
			height = maximumFrameBufferSize;
		}
		m_FrameBufferWidth = width;
		m_FrameBufferHeight = height;


		glGenFramebuffers(1, &m_FrameBufferId);

		glGenTextures(1, &m_ColourBufferAttachmentId);
		glBindTexture(GL_TEXTURE_2D, m_ColourBufferAttachmentId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_FrameBufferWidth, m_FrameBufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &m_DepthStencilBufferAttachmentId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilBufferAttachmentId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_FrameBufferWidth, m_FrameBufferHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColourBufferAttachmentId, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBufferAttachmentId);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			SERVER_FATAL("Failed to generate framebuffer - status is not complete");
			m_HadErrorBuilding = true;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer() {
		glDeleteTextures(1, &m_ColourBufferAttachmentId);
		glDeleteRenderbuffers(1, &m_DepthStencilBufferAttachmentId);
		glDeleteFramebuffers(1, &m_FrameBufferId);
	}

	void FrameBuffer::Bind() const { 
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId); 
		glViewport(0, 0, m_FrameBufferWidth, m_FrameBufferHeight);
	}
	void FrameBuffer::Unbind() const { 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//TODO: Reset glViewport to default framebuffer? Requires access to GLFW window instance
	}
	void FrameBuffer::Clear() const { 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	}

}