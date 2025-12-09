#include "hzpch.h"
#include "Platform/Vulkan/RenderPassDSM.h"

#include "Hazel/Renderer/CommandBuffer.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Platform/Vulkan/VulkanBuffer.h"

namespace Hazel {




	RenderPassDescriptorSetManager::RenderPassDescriptorSetManager(const DescriptorSetManagerSpecification& spec)
	{
		m_Specification = spec;
		Init();
	}

	void RenderPassDescriptorSetManager::Init()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		uint32_t frameInFlight = Renderer::GetFrameInFlight();
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

		Ref<VulkanShader> vulkanShader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& reflectionData = vulkanShader->GetReflectionData();
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = vulkanShader->GetDescriptorSetLayouts();

		m_WriteDescriptors.resize(frameInFlight);
		m_UniformBuffers.resize(frameInFlight);
		m_DescriptorSets.resize(frameInFlight);

		for (uint32_t frameIndex = 0; frameIndex < frameInFlight; frameIndex++) {

			for (auto& [set, bindingMap] : reflectionData) {
				

				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_DescriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &descriptorSetLayouts[set];

				VkDescriptorSet& descriptorSet = m_DescriptorSets[frameIndex][set];

				VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

			}

		}

	}

	bool RenderPassDescriptorSetManager::SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index)
	{
		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		ShaderReflectionData* rd = shader->GetReflectionDataByName(name);
		uint32_t frameInFlight = Renderer::GetFrameInFlight();		


		if (!rd) {
			HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
			return false;
		}


		uint32_t set = rd->descriptorSet;
		uint32_t binding = rd->binding;

		// todo : now only support frame in flight amount uniform buffer input
		HZ_CORE_ASSERT(uniformBufferSet->GetUniformBufferAmount() == frameInFlight, "now only support frame in flight amount uniform buffer input");

		if (m_WriteDescriptors[0].find(set) == m_WriteDescriptors[0].end() || m_WriteDescriptors[0][set].find(binding) == m_WriteDescriptors[0][set].end()) {


			for (uint32_t frameIndex = 0; frameIndex < frameInFlight; frameIndex++) {

				m_WriteDescriptors[frameIndex][set][binding] = VkWriteDescriptorSet{};

				VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
				writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptor.dstSet = m_DescriptorSets[frameIndex][set];
				writeDescriptor.dstBinding = binding;
				writeDescriptor.dstArrayElement = 0;
				writeDescriptor.descriptorCount = rd->arraySize;
				writeDescriptor.pImageInfo = nullptr;
				writeDescriptor.pTexelBufferView = nullptr;

				writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
								

				m_UniformBuffers[frameIndex][set][binding].reserve(rd->arraySize);
								
				char* tempData = new char[rd->size];
				std::memset(tempData, 0, rd->size);
				for (uint32_t i = 0; i < rd->arraySize; i++) {

					Ref<UniformBuffer> uniformBuffer = i == index ? uniformBufferSet->Get(frameIndex) : UniformBuffer::Create(rd->size, binding);
					m_UniformBuffers[frameIndex][set][binding].push_back(uniformBuffer);										
										
					if (i != index) {
						Ref<VulkanUniformBuffer> vulkanUniformBuffer = std::static_pointer_cast<VulkanUniformBuffer>(m_UniformBuffers[frameIndex][set][binding].back());
						vulkanUniformBuffer->SetData(tempData, rd->size);
					}										
				}
				delete[] tempData;
								
			}
		
			return true;
		}

		for (uint32_t frameIndex = 0; frameIndex < frameInFlight; frameIndex++) {

			Ref<UniformBuffer> uniformBuffer = uniformBufferSet->Get(frameIndex);
			m_UniformBuffers[frameIndex][set][binding][index] = uniformBuffer;
		}
								

		return true;

	}

	bool RenderPassDescriptorSetManager::SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index) {
		return false;
	}

	bool RenderPassDescriptorSetManager::SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index) {

		HZ_CORE_ASSERT(texture != nullptr, "Invalid texture!");

		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		ShaderReflectionData* rd = shader->GetReflectionDataByName(name);
		uint32_t frameInFlight = Renderer::GetFrameInFlight();
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		if (!rd) {
			HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
			return false;
		}
					
		uint32_t set = rd->descriptorSet;
		uint32_t binding = rd->binding;
		

		if (m_WriteDescriptors[frameIndex].find(set) == m_WriteDescriptors[frameIndex].end() ||
			m_WriteDescriptors[frameIndex][set].find(binding) == m_WriteDescriptors[frameIndex][set].end()) {

			
			for (uint32_t i = 0; i < frameInFlight; i++) {

				m_WriteDescriptors[i][set][binding] = VkWriteDescriptorSet{};																

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[i][set][binding];
				writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptor.dstSet = m_DescriptorSets[i][set];
				writeDescriptor.dstBinding = binding;
				writeDescriptor.dstArrayElement = 0;
				writeDescriptor.descriptorCount = rd->arraySize;
				writeDescriptor.pTexelBufferView = nullptr;				
				writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
			

			for (uint32_t i = 0; i < rd->arraySize; i++) {
				m_Textures[set][binding].push_back(Renderer::GetDefaultBlackQuadTexture());
			}
			
		}

		m_Textures[set][binding][index] = texture;

		
		return true;
	}

	void RenderPassDescriptorSetManager::Submit(){

		static VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		uint8_t frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();
		uint32_t frameInFlight = Renderer::GetFrameInFlight();

		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& reflectionDatas = shader->GetReflectionData();


		std::vector<VkWriteDescriptorSet> writeDescriptors;

		std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfosArray;
		std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfosArray;

		for (auto& [set, bindingMap] : m_UniformBuffers[frameIndex]) {

			for (auto& [binding, uniformBuffers] : bindingMap) {

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
				const ShaderReflectionData& data = reflectionDatas.at(set).at(binding);

				descriptorBufferInfosArray.push_back(std::vector<VkDescriptorBufferInfo>());
				std::vector<VkDescriptorBufferInfo>& descriptorBufferInfos = descriptorBufferInfosArray.back();
				descriptorBufferInfos.reserve(data.arraySize);

				for (auto uniformBuffer : uniformBuffers) {

					Ref<VulkanUniformBuffer> ub = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffer);																																	
					descriptorBufferInfos.push_back(ub->GetDescriptorBufferInfo());					
				}

				writeDescriptor.pBufferInfo = descriptorBufferInfos.data();
				writeDescriptors.push_back(writeDescriptor);
				

			}

		}

		for (auto& [set, bindingMap] : m_Textures) {
			for (auto& [binding, vector] : bindingMap) {

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
				descriptorImageInfosArray.push_back(std::vector<VkDescriptorImageInfo>());
				std::vector<VkDescriptorImageInfo>& imageInfos = descriptorImageInfosArray.back();

				for (auto& texture : vector) {
					Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
					imageInfos.push_back(vulkanTexture->GetDescriptorImageInfo());															
				}

				writeDescriptor.pImageInfo = imageInfos.data();
				writeDescriptors.push_back(writeDescriptor);
				
			}
		}



		vkUpdateDescriptorSets(device, writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);

	}
	

}