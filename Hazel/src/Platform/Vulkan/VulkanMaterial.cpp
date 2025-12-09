#include "hzpch.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Hazel/Renderer/Renderer.h"


namespace Hazel {
	
	VulkanMaterial::VulkanMaterial(Ref<Shader> shader) {
				
		SetShader(shader);
		
	}

	void VulkanMaterial::SetShader(Ref<Shader> shader)
	{
		m_Shader = shader;
		Ref<VulkanShader> vulkanShader = std::dynamic_pointer_cast<VulkanShader>(m_Shader);		

		HZ_CORE_ASSERT(vulkanShader != nullptr, "Invalid shader, need correct vulkan shader");


		// Initialize data
		m_Data.clear();

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& refelectionDatas = std::static_pointer_cast<VulkanShader>(m_Shader)->GetReflectionData();

		for (auto& [set, bindingMap] : refelectionDatas) {
			for (auto& [binding, data] : bindingMap) {

				const ShaderReflectionData& reflectionData = data;

				m_Data.emplace(reflectionData.name, reflectionData.size);

			}
		}

		DescriptorSetManagerSpecification dsmSpec;
		dsmSpec.shader = m_Shader;
		dsmSpec.usage = DescriptorSetManagerUsage::Material;

		m_DescriptorSetManager = DescriptorSetManager::Create(dsmSpec);		

	}


	const std::unordered_map<uint32_t, VkDescriptorSet>& VulkanMaterial::GetDescriptorSets(uint32_t frameIndex) const
	{
		return m_DescriptorSetManager->GetDescriptorSets(frameIndex);
	}

	// todo 
	void VulkanMaterial::Submit()
	{
		// todo 
		//Renderer::GetCurrentFrameIndex();

		m_DescriptorSetManager->Submit();
		

	}
	
}



