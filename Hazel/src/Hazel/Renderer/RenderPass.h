#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/ByteKey.h"

#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Resource/Resource.h"
#include "Hazel/Resource/ResourceManager.h"

namespace Hazel {

	class Pipeline;
	class UniformBufferSet;
	class UniformBuffer;
	class Texture2D;
	class Framebuffer;

	struct RenderPassInfo {

		std::vector<AttachmentInfo> attachmentInfos;

		Handle<Framebuffer> framebuffer;

		glm::vec4 renderArea;

	};


	class RenderPass {
	public:
		RenderPass() = default;
		RenderPass(const RenderPassInfo& info);
		virtual ~RenderPass() = default;

		RenderPassInfo& GetInfo() { return m_Info; }

		static Ref<RenderPass> Create(const RenderPassInfo& info);

		// Render pass input can't not be set by direct value
		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) = 0;
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) = 0;
		virtual bool SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0) = 0;
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) = 0;

		virtual void SetFramebuffer(const Handle<Framebuffer>& framebuffer, glm::vec4 renderArea = {}) {
			auto framebufferRef = ResourceManager<Framebuffer>::Get(framebuffer);
			m_Info.framebuffer = framebuffer;
			if (renderArea == glm::vec4()) {
				m_Info.renderArea = { 0.0f, 0.0f, framebufferRef->GetInfo().width, framebufferRef->GetInfo().height };
			}
		}


		// submit data to descriptor
		virtual void Submit() = 0;

	protected:

		RenderPassInfo m_Info;
	};

}
