#pragma once

#include "Hazel/Renderer/Material.h"

#include <vulkan/vulkan.h>


namespace Hazel {

	class VulkanUniformBuffer;
	class VulkanShader;

	class VulkanMaterial : public Material{


	public:

		// todo remove this
		VkPipelineLayout pipelineLayout;


		VulkanMaterial(Ref<Shader> shader);
				
		void Set(const std::string& name, glm::mat3 matrix);
		void Set(const std::string& name, glm::mat4 matrix);		
		
		template<typename T>
		void Set(const std::string& name, const T& value) {

			ShaderReflectionData* rd = m_VulkanShader->GetReflectionDataByName(name);

			HZ_CORE_ASSERT(rd, "Could not find uniform named {}", name);
			
			if (!rd)
				return;

			static uint32_t frameInFlight = Renderer::GetFrameInFlight();
			for (int i = 0; i < frameInFlight; i++) {
				Ref<VulkanUniformBuffer> ub = m_UniformBuffers[i][rd->descriptorSet][rd->binding];
				ub->SetData(&value, sizeof(T));
			}			
		}

		std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }

	private:

		

		Ref<VulkanShader> m_VulkanShader;

		//[frame][set]
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets; 
		std::unordered_map<std::string, VkWriteDescriptorSet> m_WriteDescriptorSets;
		
		// [frame][set][binding]
		//std::vector<std::vector<std::vector<Ref<VulkanUniformBuffer>>>> m_UniformBuffers;
		// [frame] set -> binding -> uniformBufer
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>>> m_UniformBuffers;

		// [frame] set -> binding -> VkWriteDescriptorSet
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t,VkWriteDescriptorSet>>> m_WriteDescriptors;

		VkDescriptorPool m_DescriptorPool;

		
	};


}