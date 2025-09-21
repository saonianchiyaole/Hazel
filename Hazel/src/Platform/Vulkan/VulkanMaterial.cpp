#include "hzpch.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Hazel/Renderer/Renderer.h"


namespace Hazel {

	Ref<Texture2D> VulkanMaterial::s_DefaultBlackQuad = nullptr;


	VulkanMaterial::VulkanMaterial(Ref<Shader> shader) {

		if (s_DefaultBlackQuad == nullptr) {
			//s_DefaultBlackQuad = Texture2D::Create("");
			uint32_t black = 0;
			s_DefaultBlackQuad = MakeRef<VulkanTexture2D>(TextureFormat::RGBA, 1, 1);
			s_DefaultBlackQuad->SetData(&black, sizeof(uint32_t));
		}

		m_UniformBuffers.clear();
		m_WriteDescriptors.clear();

		m_DescriptorSets.clear();
		
		SetShader(shader);

		
	}

	void VulkanMaterial::SetShader(Ref<Shader> shader)
	{
		m_Shader = shader;
		Ref<VulkanShader> vulkanShader = std::dynamic_pointer_cast<VulkanShader>(m_Shader);
		m_VulkanShader = vulkanShader;

		HZ_CORE_ASSERT(vulkanShader != nullptr, "Invalid shader, need correct vulkan shader");


		// Initialize data
		m_Data.clear();

		const std::vector<std::vector<ShaderReflectionData>>& refelecttionDatas = m_VulkanShader->GetReflectionData();

		for (uint32_t set = 0; set < refelecttionDatas.size(); set++) {
			for (uint32_t binding = 0; binding < refelecttionDatas[set].size(); binding++) {

				const ShaderReflectionData& reflectionData = refelecttionDatas[set][binding];

				m_Data.emplace(reflectionData.name, reflectionData.size);

			}
		}

		

		//

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		// Create Descriptor Pool
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10 * 3; // frames in flight should partially determine this
		poolInfo.poolSizeCount = 10;
		poolInfo.pPoolSizes = poolSizes;

		HZ_CORE_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");


		const std::vector<std::vector<ShaderReflectionData>>& reflectionData = vulkanShader->GetReflectionData();
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = vulkanShader->GetDescriptorSetLayouts();

		uint32_t frameInFlight = Renderer::GetFrameInFlight();

		for (int i = 0; i < frameInFlight; i++) {

			m_DescriptorSets.emplace_back();

		}

		for (uint32_t set = 0; set < reflectionData.size(); set++) {

			for (uint32_t frameIndex = 0; frameIndex < frameInFlight; frameIndex++) {


				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_DescriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &descriptorSetLayouts[set];

				m_DescriptorSets[frameIndex].emplace_back();

				VkDescriptorSet& descriptorSet = m_DescriptorSets[frameIndex][set];

				HZ_CORE_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) == VK_SUCCESS, "Failed to allocate descriptorSets");

				m_WriteDescriptors.emplace_back();
				m_UniformBuffers.emplace_back();
				m_Textures.emplace_back();

				for (uint32_t binding = 0; binding < reflectionData[set].size(); binding++) {

					const ShaderReflectionData& data = reflectionData[set][binding];

					m_WriteDescriptors[frameIndex][set][binding] = VkWriteDescriptorSet{};

					VkWriteDescriptorSet& writeDesciptor = m_WriteDescriptors[frameIndex][set][binding];
					writeDesciptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDesciptor.dstSet = m_DescriptorSets[frameIndex][set];
					writeDesciptor.dstBinding = binding;
					writeDesciptor.dstArrayElement = 0;
					writeDesciptor.descriptorCount = 1;
					writeDesciptor.pImageInfo = nullptr;
					writeDesciptor.pTexelBufferView = nullptr;

					switch (data.type) {
					case DescriptorType::UniformBuffer:
					{
						writeDesciptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						m_UniformBuffers[frameIndex][set][binding] = MakeRef<VulkanUniformBuffer>(data.size, binding);
						writeDesciptor.pBufferInfo = &m_UniformBuffers[frameIndex].at(set).at(binding)->GetDescriptorBufferInfo();
						// Initialize buffer with empty data
						char* tempData = new char[data.size];
						std::memset(tempData, 0, data.size);
						m_UniformBuffers[frameIndex][set][binding]->SetData(tempData, data.size);
						delete[] tempData;						
						break;
					}
					case DescriptorType::StorageBuffer:
						break;
					case DescriptorType::Sampler2D:
						writeDesciptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						Ref<VulkanTexture2D> texture = std::dynamic_pointer_cast<VulkanTexture2D>(VulkanMaterial::s_DefaultBlackQuad);
						m_Textures[frameIndex][set][binding] = texture;
						if (!texture)
							continue;
						VkDescriptorImageInfo& imageInfo = texture->GetDescriptorImageInfo();
						writeDesciptor.pImageInfo = &imageInfo;
						break;
					}

					vkUpdateDescriptorSets(device, 1, &writeDesciptor, 0, nullptr);
				}

			}

		}

	}


	// todo 
	void VulkanMaterial::Submit()
	{
		// todo 
		//Renderer::GetCurrentFrameIndex();

		static VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		
		uint8_t frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();		

		const std::vector<std::vector<ShaderReflectionData>>& reflectionDatas = m_VulkanShader->GetReflectionData();

		for (uint32_t set = 0; set < reflectionDatas.size(); set++) {
			for (uint32_t binding = 0; binding < reflectionDatas[set].size(); binding++) {

				const ShaderReflectionData& reflectionData = reflectionDatas[set][binding];

				switch (reflectionDatas[set][binding].type) {
				case DescriptorType::Sampler2D:
				{
					VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
					writeDescriptor.pImageInfo = &std::static_pointer_cast<VulkanTexture2D>(m_Textures[frameIndex][set][binding])->GetDescriptorImageInfo();
					vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);
					break;
				}
				case DescriptorType::UniformBuffer:
				{
					Ref<VulkanUniformBuffer> ub = m_UniformBuffers[frameIndex][set][binding];
					ub->SetData(m_Data[reflectionData.name], reflectionDatas[set][binding].size);
					break;
				}
				}


			}
		}


	}
	
}



