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

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual void ClearAttachment(uint32_t attachmentIndex, const void* value) override;

		virtual const uint32_t GetColorAttachment(int index = 0) override;
		virtual const uint32_t GetDpethAttachment() override;
		virtual const void BindTexture(uint32_t index, uint32_t slot = 0) override;

		virtual void Bind() override;
		virtual void Unbind() override;
	private:
		uint32_t m_RendererID;
		
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureFormat> m_ColorAttachmentFormats;
		std::vector<uint32_t> m_ColorAttachments; 
		uint32_t m_DepthAttachment = 0;
		FramebufferTextureFormat m_DepthAttachmentFormat = FramebufferTextureFormat::None;
	};
}