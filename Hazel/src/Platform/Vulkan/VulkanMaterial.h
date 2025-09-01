#pragma once

#include "Hazel/Renderer/Material.h"

#include <vulkan/vulkan.h>
#include <Platform/Vulkan/VulkanShader.h>
#include <Platform/Vulkan/VulkanTexture.h>
#include <Platform/Vulkan/VulkanContext.h>
#include <Platform/Vulkan/VulkanDevice.h>
#include <Hazel/Renderer/Renderer.h>

namespace Hazel {

	class VulkanUniformBuffer;
	class VulkanTexture2D;
	class VulkanShader;

	class VulkanMaterial : public Material{


	public:
		
		VulkanMaterial(Ref<Shader> shader);
								
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

		template<>
		void VulkanMaterial::Set(const std::string& name, const Ref<Texture2D>& texture) {

			HZ_CORE_ASSERT(texture != nullptr, "Empty texture 2D!", name);

			ShaderReflectionData* rd = m_VulkanShader->GetReflectionDataByName(name);
			
			HZ_CORE_ASSERT(rd, "Could not find texture uniform named {}", name);
									
			VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

			Ref<VulkanTexture2D> vulkanTexture = std::dynamic_pointer_cast<VulkanTexture2D>(texture);
			static uint32_t frameInFlight = Renderer::GetFrameInFlight();
			for (int i = 0; i < frameInFlight; i++) {
				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[i][rd->descriptorSet][rd->binding];
				writeDescriptor.pImageInfo = &vulkanTexture->GetDescriptorImageInfo();
				vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);
			}
		}

		std::vector<std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_DescriptorSets; }


		virtual void Submit() override;

	private:

		
		Ref<VulkanShader> m_VulkanShader;

		//[frame][set]
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets; 
		std::unordered_map<std::string, VkWriteDescriptorSet> m_WriteDescriptorSets;
		
		// [frame][set][binding]
		//std::vector<std::vector<std::vector<Ref<VulkanUniformBuffer>>>> m_UniformBuffers;
		
		// [frame] set -> binding -> uniformBufer
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>>> m_UniformBuffers;
		// [frame] set -> binding -> imageBuffer
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, Ref<Texture>>>> m_Samplers;

		// [frame] set -> binding -> VkWriteDescriptorSet
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkWriteDescriptorSet>>> m_WriteDescriptors;

		VkDescriptorPool m_DescriptorPool;


		static Ref<Texture2D> s_DefaultBlackQuad;
		
	};


}