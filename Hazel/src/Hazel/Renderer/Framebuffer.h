#pragma once



#include "hzpch.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Texture.h"
#include "glm/glm.hpp"


namespace Hazel {

	namespace Utils {

		bool IsDepthFormat(PixelFormat format);

	}


	struct AttachmentInfo {
		
		
		PixelFormat format;

		bool isClearColor = false;


		union ClearValue{
			glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

			struct {
				float depth;
				uint32_t stencil;
			private:
				float padding[2];
			}depthStencil;

		};
		
		uint8_t samples = 1;

		ClearValue clearValue;

		AttachmentInfo(PixelFormat _format) : format(_format){}

	};

	struct FramebufferInfo {

		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		std::vector<AttachmentInfo> attachments;

		bool swapChainTarget = false;
	};


	class Texture2D;
	class RenderPass;

	class Framebuffer {
	public:
		
		virtual const			FramebufferInfo& GetInfo() { return m_Info; };
		virtual void			Invalidate() {};
		virtual void			Resize(const FramebufferInfo& spec) {};
		virtual void			Resize(const glm::vec2 size) {};

		virtual void			WaitRenderFinished() {};

		virtual void			Bind() {};
		virtual void			Unbind() {};
		
		virtual int				ReadPixel(uint32_t attachmentIndex, int x, int y) { return 0; };
		virtual void			ClearAttachment(uint32_t attachmentIndex, const void* value = nullptr) {};
		virtual void			ClearAllAttachments() {};
		virtual const void		BindTexture(uint32_t index, uint32_t slot = 0) {};

		virtual const std::vector<Ref<Texture2D>>	GetColorAttachments()				{ return m_ColorAttachments; }
		virtual const Ref<Texture2D>				GetColorAttachment(int index = 0)	{ HZ_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of range");  return m_ColorAttachments[index]; }
		virtual const Ref<Texture2D>				GetDpethAttachment()				{ return m_DepthAttachment; };


		static Ref<Framebuffer> Create(const FramebufferInfo& spec);

	protected:


		FramebufferInfo	m_Info;

		Ref<Texture2D>				m_DepthAttachment = nullptr;
		std::vector<Ref<Texture2D>> m_ColorAttachments;

		Ref<RenderPass>				m_RenderPass;

	};

}
