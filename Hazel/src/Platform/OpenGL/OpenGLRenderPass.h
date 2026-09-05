#pragma once

#include "Hazel/Renderer/RenderPass.h"

namespace Hazel {



	class OpenGLRenderPass : public RenderPass {

	public:

		OpenGLRenderPass(const RenderPassInfo& spec);

		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) { return true; };
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) { return true; };
		virtual bool SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0) { return true; };
		virtual void Submit() override {};
	private:

	};


}
