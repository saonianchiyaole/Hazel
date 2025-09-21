#pragma once
#include "Hazel/Renderer/Framebuffer.h"


namespace Hazel {

	class OpenGLFramebuffer : public Framebuffer{
	public:

		OpenGLFramebuffer(const FramebufferSpecification& spec);	
		~OpenGLFramebuffer();
		virtual const FramebufferSpecification& GetSpecification() override;
		virtual void Invalidate() override;
		virtual void Resize(const FramebufferSpecification& spec) override;
		virtual void Resize(const glm::vec2 size) override;

		virtual int			ReadPixel(uint32_t index, int x, int y) override;
		virtual void		ClearAttachment(uint32_t index, const void* value) override;
		virtual const void	BindTexture(uint32_t index, uint32_t slot = 0) override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetColorAttachmentID(uint32_t index) { return m_ColorAttachmentIDs[index]; }
		virtual uint32_t GetDepthAttachmentID() { return m_DepthAttachmentID; }

	private:
		uint32_t m_RendererID;
				
		std::vector<TextureFormat> m_ColorAttachmentFormats;
		std::vector<uint32_t> m_ColorAttachmentIDs; 
		uint32_t m_DepthAttachmentID = 0;
		TextureFormat m_DepthAttachmentFormat = TextureFormat::None;
	};
}