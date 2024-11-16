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

		virtual const uint32_t GetColorAttachment() override;
		virtual const uint32_t GetDpethAttachment() override;


		virtual void Bind() override;
		virtual void Unbind() override;
	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FramebufferSpecification m_Specification;
	};
}