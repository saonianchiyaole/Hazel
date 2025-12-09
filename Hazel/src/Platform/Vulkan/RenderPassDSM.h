#pragma once
#include "Platform/Vulkan/DescriptorSetManager.h"

namespace Hazel {



	class RenderPassDescriptorSetManager : public DescriptorSetManager {


	public:

		RenderPassDescriptorSetManager(const DescriptorSetManagerSpecification& spec);
		

		virtual void Init() override;
		

		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) override;
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) override;
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0) override;

		virtual void Submit() override;

	};




}

