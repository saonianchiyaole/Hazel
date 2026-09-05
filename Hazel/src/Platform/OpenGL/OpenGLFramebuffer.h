#pragma once
#include "Hazel/Renderer/Framebuffer.h"


namespace Hazel {

	class OpenGLFramebuffer : public Framebuffer{
	public:

		OpenGLFramebuffer(const FramebufferInfo& info);
		~OpenGLFramebuffer();
		virtual const FramebufferInfo& GetInfo() override;
		virtual void Invalidate		() override;
		virtual void Resize			(const FramebufferInfo& info) override;
		virtual void Resize			(const glm::vec2 size) override;

		virtual void		WaitRenderFinished() override;

		virtual int			ReadPixel				(uint32_t index, int x, int y) override;
		virtual void		ClearAttachment			(uint32_t index, const void* value) override;
		virtual void		ClearAllAttachments		() override;
		virtual const void	BindTexture				(uint32_t index, uint32_t slot = 0) override;

		virtual void		Bind() override;
		virtual void		Unbind() override;

		virtual uint32_t	GetColorAttachmentID	(uint32_t index)	{ return m_ColorAttachmentIDs[index]; }
		virtual uint32_t	GetDepthAttachmentID	()					{ return m_DepthAttachmentID; }

	private:
		uint32_t m_RendererID;
				
		std::vector<PixelFormat> m_ColorAttachmentFormats;
		std::vector<uint32_t> m_ColorAttachmentIDs; 
		uint32_t m_DepthAttachmentID = 0;
		PixelFormat m_DepthAttachmentFormat = PixelFormat::None;
	};
}