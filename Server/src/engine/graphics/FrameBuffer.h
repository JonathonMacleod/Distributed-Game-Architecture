#ifndef SERVER__FRAMEBUFFER__H
	#define SERVER__FRAMEBUFFER__H

	#include "Graphics.h"

	namespace Server::Engine::Graphics {

		class FrameBuffer {
			public:
				FrameBuffer(int width = -1, int height = -1);
				~FrameBuffer();

				void Bind() const;
				void Unbind() const;
				void Clear() const;

				GLsizei GetWidth() const { return m_FrameBufferWidth; }
				GLsizei GetHeight() const { return m_FrameBufferHeight; }

				GLuint GetFrameBufferId() const { return m_FrameBufferId; }
				GLuint GetColourBufferAttachmentTextureId() const { return m_ColourBufferAttachmentId; }
				GLuint GetDepthAndStencilBufferAttachmentRenderBufferId() const { return m_DepthStencilBufferAttachmentId; }

				bool HadErrorBuilding() const { return m_HadErrorBuilding; }

			private:
				bool m_HadErrorBuilding = false;

				GLuint m_FrameBufferId;
				GLuint m_ColourBufferAttachmentId;
				GLuint m_DepthStencilBufferAttachmentId;

				GLsizei m_FrameBufferWidth;
				GLsizei m_FrameBufferHeight;
		};
	
	}

#endif