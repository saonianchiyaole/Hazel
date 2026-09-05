#include "hzpch.h"



#include "Hazel/Renderer/CommandBuffer.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Platform/Vulkan/VulkanBuffer.h"

#include "glm/glm.hpp"

#include "Hazel/Renderer/Camera.h"

namespace Hazel {
	
	struct UniformBufferObject {

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

	};


	Scope<DescriptorSetManager> DescriptorSetManager::Create(const DescriptorSetManagerInfo& info) {


		if (info.device == nullptr) {
			return nullptr;
		}

		return MakeScope<DescriptorSetManager>(info);				

	}




	DescriptorSetManager::DescriptorSetManager(const DescriptorSetManagerInfo& info)
		: m_Info(info)
	{

		HZ_CORE_ASSERT(m_Info.device, "Invalid device!");
		Init();		
	}

	DescriptorSetManager::~DescriptorSetManager()
	{
	}

	void DescriptorSetManager::Init()
	{

		VkDevice device = m_Info.device->GetRawDevice();

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
		poolInfo.maxSets = 10;
		poolInfo.poolSizeCount = 10;
		poolInfo.pPoolSizes = poolSizes;

		HZ_CORE_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");

		m_DescriptorSets.clear();
		m_UniformBuffers.clear();
		m_Textures.clear();
		m_WriteDescriptors.clear();

		Ref<VulkanDevice> vulkanDevice = VulkanContext::GetCurrentContext()->GetDevice();
		
		
	}

	void DescriptorSetManager::Prepare(Handle<Shader> shader)
	{

		if (shader == m_BakedShader) {
			return;
		}

		m_DescriptorSets.clear();
		m_UniformBuffers.clear();
		m_Textures.clear();
		m_WriteDescriptors.clear();

		VkDevice device = m_Info.device->GetRawDevice();
		Ref<VulkanShader> vulkanShader = m_Info.device->GetProxy(shader);
		HZ_CORE_ASSERT(vulkanShader, "Invalid Vulkan shader handle in DescriptorSetManager");

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& reflectionData = vulkanShader->GetSnapshot()->relectionData;
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = vulkanShader->GetDescriptorSetLayouts();

		std::vector<VkWriteDescriptorSet> writeDescriptors;
		std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfosArray;
		std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfosArray;

		for (auto& [set, bindingMap] : reflectionData) {

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &descriptorSetLayouts[set];

			VkDescriptorSet& descriptorSet = m_DescriptorSets[set];

			VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

			for (auto& [binding, data] : bindingMap) {

				m_WriteDescriptors[set][binding] = VkWriteDescriptorSet{};

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[set][binding];
				writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptor.dstSet = m_DescriptorSets[set];
				writeDescriptor.dstBinding = binding;
				writeDescriptor.dstArrayElement = 0;
				writeDescriptor.descriptorCount = data.arraySize;
				writeDescriptor.pImageInfo = nullptr;
				writeDescriptor.pTexelBufferView = nullptr;

				switch (data.type) {
				case DescriptorType::UniformBuffer:
				{
					writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

					m_UniformBuffers[set][binding].reserve(data.arraySize);

					descriptorBufferInfosArray.push_back(std::vector<VkDescriptorBufferInfo>());
					std::vector<VkDescriptorBufferInfo>& descriptorBufferInfos = descriptorBufferInfosArray.back();
					descriptorBufferInfos.reserve(data.arraySize);

					char* tempData = new char[data.size];
					std::memset(tempData, 0, data.size);
					for (uint32_t i = 0; i < data.arraySize; i++) {

						Ref<VulkanUniformBuffer> uniformBuffer = MakeRef<VulkanUniformBuffer>(data.size);
						m_UniformBuffers[set][binding].push_back(uniformBuffer);						
						uniformBuffer->SetData(tempData, data.size);
						descriptorBufferInfos.push_back(uniformBuffer->GetDescriptorBufferInfo());

					}

					delete[] tempData;

					writeDescriptor.pBufferInfo = descriptorBufferInfos.data();
					// Initialize buffer with empty data


					break;
				}
				case DescriptorType::StorageBuffer:
					break;
				case DescriptorType::Sampler2D:
				{
					writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

					m_Textures[set][binding].reserve(data.arraySize);
					descriptorImageInfosArray.push_back(std::vector<VkDescriptorImageInfo>());
					std::vector<VkDescriptorImageInfo>& imageInfos = descriptorImageInfosArray.back();
					imageInfos.reserve(data.arraySize);


					Handle<Texture2D> defaultTexture2DHandle = Renderer::GetDefaultBlackQuadTexture();
					Ref<VulkanTexture2D> defaultTexture2D = m_Info.device->GetProxy(defaultTexture2DHandle);
					HZ_CORE_ASSERT(defaultTexture2D, "Failed to resolve default Vulkan texture proxy");

					for (uint32_t i = 0; i < data.arraySize; i++) {

						m_Textures[set][binding].push_back(defaultTexture2D);
						imageInfos.push_back(defaultTexture2D->GetDescriptorImageInfo());

					}

					// Default texture						

					writeDescriptor.pImageInfo = imageInfos.data();

					break;
				}
				}

				writeDescriptors.push_back(writeDescriptor);

			}

		}

		// collect all writeDescriptor



		vkUpdateDescriptorSets(device, writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}


	bool DescriptorSetManager::SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index) {
		return true;
	}

	bool DescriptorSetManager::SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index) {


		

		Ref<VulkanTexture2D> textureProxy = m_Info.device->GetProxy(texture);
		HZ_CORE_ASSERT(textureProxy, "Invalid Vulkan texture handle!");

		Ref<VulkanShader> shader = m_Info.device->GetProxy(m_BakedShader);
		HZ_CORE_ASSERT(shader, "Invalid Vulkan shader handle in DescriptorSetManager");

		Ref<const ShaderSnapshot> snapshot = shader->GetSnapshot();

		if (snapshot->relectionDataByName.find(name) == snapshot->relectionDataByName.end()) {
			HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
			return false;
		}
		const ShaderReflectionData& rd = snapshot->relectionDataByName.at(name);

		
		HZ_CORE_ASSERT(index < rd.arraySize, "Shaer: {},\n uniform : {}\n, Index : {} out of range, array size : {}!",
			snapshot->name, name, index, rd.arraySize);


		m_Textures[rd.descriptorSet][rd.binding][index] = textureProxy;

		return true;

	}

	bool DescriptorSetManager::SetData(Ref<UniformCache> uniformCache)
	{
		return false;
	}

	std::vector<VkDescriptorSet> DescriptorSetManager::GetSortedDescriptorSets(const std::unordered_map<uint32_t, VkDescriptorSet>& descriptorSets)
	{

		std::vector<std::pair<uint32_t, VkDescriptorSet>> temp(descriptorSets.begin(), descriptorSets.end());

		std::sort(temp.begin(), temp.end(),
			[](const auto& a, const auto& b) { return a.first < b.first; }
		);


		std::vector<VkDescriptorSet> result;
		result.reserve(temp.size());

		for (auto& [set, vkDescriptorSet] : temp) {
			result.push_back(vkDescriptorSet);
		}

		return result;
	}
	
	const std::unordered_map<uint32_t, VkDescriptorSet>& DescriptorSetManager::GetDescriptorSets() const
	{
		return m_DescriptorSets;
	}

	std::vector<VkDescriptorSet> DescriptorSetManager::GetSortedDescriptorSets()
	{
		return GetSortedDescriptorSets(m_DescriptorSets);
	}

}



