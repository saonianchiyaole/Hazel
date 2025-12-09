#pragma once

#include "Hazel/Renderer/RenderPass.h"

namespace Hazel {



	class OpenGLRenderPass : public RenderPass {

	public:

		OpenGLRenderPass(const RenderPassSpecification& spec);

		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) { return true; };
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) { return true; };
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) { return true; };
		virtual void Submit() override {};
	private:

	};


}