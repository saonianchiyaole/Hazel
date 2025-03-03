#pragma once



#include "hzpch.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "glm/glm.hpp"

namespace Hazel {



	enum class FramebufferTextureFormat {
		None = 0,
		RGBA8,
		DEPTH24STENCIL8,
		RED_INTEGER,
		Depth = DEPTH24STENCIL8
	};



	struct FramebufferTextureSpecification {
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat formatVal) :
			textureFormat(formatVal) {}
		FramebufferTextureFormat textureFormat;
	};

	struct FramebufferAttachmentSpecification {
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> attachmentsList)
			: attachments(attachmentsList){}
		std::vector<FramebufferTextureSpecification> attachments;
	};

	struct FramebufferSpecification {
		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		FramebufferAttachmentSpecification attachments;

		bool SwapChainTarget = false;
	};

	class Framebuffer {
	public:

		virtual const FramebufferSpecification& GetSpecification() = 0;
		virtual void Invalidate() = 0;
		virtual void Resize(const FramebufferSpecification& spec) = 0;
		virtual void Resize(const glm::vec2 size) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, const void* value) = 0;
		virtual const void BindTexture(uint32_t index, uint32_t slot = 0) = 0;

		virtual const uint32_t GetColorAttachment(int index = 0) = 0;
		virtual const uint32_t GetDpethAttachment() = 0;


		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	protected:

	};

}