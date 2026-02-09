#pragma once



#include "hzpch.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Texture.h"
#include "glm/glm.hpp"


namespace Hazel {

	namespace Utils {

		bool IsDepthFormat(TextureFormat format);

	}


	struct FramebufferAttachmentSpecification {
		
		
		TextureFormat format;


		FramebufferAttachmentSpecification(TextureFormat _format) : format(_format){}

	};

	struct FramebufferSpecification {

		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		std::vector<FramebufferAttachmentSpecification> attachments;

		bool swapChainTarget = false;
	};


	class Texture2D;
	class RenderPass;

	class Framebuffer : public Asset {
	public:
		
		virtual const			FramebufferSpecification& GetSpecification() { return m_Specification; };
		virtual void			Invalidate() = 0;
		virtual void			Resize(const FramebufferSpecification& spec) = 0;
		virtual void			Resize(const glm::vec2 size) = 0;
		virtual void			Bind() = 0;
		virtual void			Unbind() = 0;
		
		virtual int				ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void			ClearAttachment(uint32_t attachmentIndex, const void* value) = 0;
		virtual const void		BindTexture(uint32_t index, uint32_t slot = 0) = 0;

		virtual const std::vector<Ref<Texture2D>>	GetColorAttachments()				{ return m_ColorAttachments; }
		virtual const Ref<Texture2D>				GetColorAttachment(int index = 0)	{ HZ_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of range");  return m_ColorAttachments[index]; }
		virtual const Ref<Texture2D>				GetDpethAttachment()				{ return m_DepthAttachment; };


		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	protected:


		FramebufferSpecification	m_Specification;

		Ref<Texture2D>				m_DepthAttachment = nullptr;
		std::vector<Ref<Texture2D>> m_ColorAttachments;

		Ref<RenderPass>				m_RenderPass;

	};

}