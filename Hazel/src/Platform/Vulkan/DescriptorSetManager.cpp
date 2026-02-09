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

#include "Platform/Vulkan/RenderPassDSM.h"

namespace Hazel {



	struct UniformBufferObject {

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

	};


	Scope<DescriptorSetManager> DescriptorSetManager::Create(const DescriptorSetManagerSpecification& spec) {

		switch (spec.usage) {

		case DescriptorSetManagerUsage::RenderPass:
			return MakeScope<RenderPassDescriptorSetManager>(spec);
		case DescriptorSetManagerUsage::Material:
			return MakeScope<DescriptorSetManager>(spec);

		}

	}




	DescriptorSetManager::DescriptorSetManager(const DescriptorSetManagerSpecification& specification)
		: m_Specification(specification)
	{

		Init();

	}

	DescriptorSetManager::~DescriptorSetManager()
	{
	}

	void DescriptorSetManager::Init()
	{

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


		uint32_t frameInFlight = Renderer::GetFrameInFlight();



		Ref<VulkanShader> vulkanShader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderReflectionData>>& reflectionData = vulkanShader->GetReflectionData();
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = vulkanShader->GetDescriptorSetLayouts();

		m_DescriptorSets.resize(frameInFlight);

		std::vector<VkWriteDescriptorSet> writeDescriptors;
		std::vector<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfosArray;
		std::vector<std::vector<VkDescriptorImageInfo>> descriptorImageInfosArray;

		for (uint32_t frameIndex = 0; frameIndex < frameInFlight; frameIndex++) {

			m_WriteDescriptors.emplace_back();
			m_UniformBuffers.emplace_back();

			for (auto& [set, bindingMap] : reflectionData) {


				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_DescriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &descriptorSetLayouts[set];

				VkDescriptorSet& descriptorSet = m_DescriptorSets[frameIndex][set];

				VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
								
				for (auto& [binding, data] : bindingMap) {
					
					m_WriteDescriptors[frameIndex][set][binding] = VkWriteDescriptorSet{};

					VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
					writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptor.dstSet = m_DescriptorSets[frameIndex][set];
					writeDescriptor.dstBinding = binding;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.descriptorCount = data.arraySize;
					writeDescriptor.pImageInfo = nullptr;
					writeDescriptor.pTexelBufferView = nullptr;

					switch (data.type) {
					case DescriptorType::UniformBuffer:
					{
						writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

						m_UniformBuffers[frameIndex][set][binding].reserve(data.arraySize);

						descriptorBufferInfosArray.push_back(std::vector<VkDescriptorBufferInfo>());
						std::vector<VkDescriptorBufferInfo>& descriptorBufferInfos = descriptorBufferInfosArray.back();
						descriptorBufferInfos.reserve(data.arraySize);

						char* tempData = new char[data.size];
						std::memset(tempData, 0, data.size);
						for (uint32_t i = 0; i < data.arraySize; i++) {

							m_UniformBuffers[frameIndex][set][binding].push_back(UniformBuffer::Create(data.size, binding));

							Ref<VulkanUniformBuffer> vulkanUniformBuffer = std::static_pointer_cast<VulkanUniformBuffer>(m_UniformBuffers[frameIndex][set][binding].back());
																					
							vulkanUniformBuffer->SetData(tempData, data.size);

							descriptorBufferInfos.push_back(vulkanUniformBuffer->GetDescriptorBufferInfo());							

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


						Ref<VulkanTexture2D> defaultTexture2D = std::static_pointer_cast<VulkanTexture2D>(Renderer::GetDefaultBlackQuadTexture());

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

		}

		// collect all writeDescriptor

				

		vkUpdateDescriptorSets(device, writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);


	}



	bool DescriptorSetManager::SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index) {

		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		ShaderReflectionData* rd = shader->GetReflectionDataByName(name);
		uint32_t frameInFlight = Renderer::GetFrameInFlight();
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();


		if (!rd) {
			HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
			return false;
		}


		uint32_t set = rd->descriptorSet;
		uint32_t binding = rd->binding;

		// todo : now only support frame in flight amount uniform buffer input
		HZ_CORE_ASSERT(uniformBufferSet->GetUniformBufferAmount() == frameInFlight, "now only support frame in flight amount uniform buffer input");

		for (uint32_t i = 0; i < frameInFlight; i++) {
			m_UniformBuffers[i][set][binding][index] = uniformBufferSet->Get(i);
		}

		return true;
	}

	bool DescriptorSetManager::SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index) {
		return true;
	}

	bool DescriptorSetManager::SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index) {


		HZ_CORE_ASSERT(texture != nullptr, "Invalid texture!");

		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		ShaderReflectionData* rd = shader->GetReflectionDataByName(name);
		uint32_t frameInFlight = Renderer::GetFrameInFlight();
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();

		if (!rd) {
			HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
			return false;
		}

		HZ_CORE_ASSERT(index <= rd->arraySize, "Shaer: {},\n uniform : {}\n, Index : {} out of range, array size : {}!",
			m_Specification.shader->GetName(), name, index, rd->arraySize);


		// todo: now only have one real texture, maybe should change in the future

		for (uint32_t i = 0; i < frameInFlight; i++) {
			m_Textures[rd->descriptorSet][rd->binding][index] = texture;
		}

		return true;

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


	void DescriptorSetManager::Submit() {

		/*static VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		uint8_t frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();
		uint32_t frameInFlight = Renderer::GetFrameInFlight();

		Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		const std::vector<std::vector<ShaderReflectionData>>& reflectionDatas = shader->GetReflectionData();


		std::vector<VkWriteDescriptorSet&> writeDescriptors;*/

		// has nothing to do in vulkan
		/*for (auto [set, bindingMap] : m_UniformBuffers[frameIndex]) {

			for (auto [binding, uniformBuffer] : bindingMap) {

				Ref<VulkanUniformBuffer> ub = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffer);

			}

		}*/

		// three frame index has the same texture, so just use the first one :(
		/*for (auto [set, bindingMap] : m_Textures) {

			for (auto [binding, textures] : bindingMap) {

				VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];

				std::vector<const VkDescriptorImageInfo&> imageInfos;
				imageInfos.reserve(writeDescriptor.descriptorCount);

				for (auto texture : textures) {

					Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);

					
					imageInfos.push_back(vulkanTexture->GetDescriptorImageInfo());

				}


				writeDescriptors.push_back(writeDescriptor);
				

			}

		}*/

		/*for (uint32_t set = 0; set < reflectionDatas.size(); set++) {
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
		}*/


		//vkUpdateDescriptorSets(device, writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);

	}

	void DescriptorSetManager::Submit(MaterialDataMap& data)
	{

		//static VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		//uint8_t frameIndex = Application::GetInstance().GetWindow().GetSwapchain()->GetCurrentFrameIndex();
		//uint32_t frameInFlight = Renderer::GetFrameInFlight();

		//Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

		//const std::vector<std::vector<ShaderReflectionData>>& reflectionDatas = shader->GetReflectionData();



		//// has nothing to do in vulkan
		//for (auto [set, bindingMap] : m_UniformBuffers[frameIndex]) {

		//	for (auto [binding, uniformBuffers] : bindingMap) {


		//		for (auto uniformBuffer : uniformBuffers) {

		//			Ref<VulkanUniformBuffer> vulkanUniformBuffer = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffer);
		//			vulkanUniformBuffer->SetData();

		//		}
		//						

		//		const std::string& uniformName = reflectionDatas[set][binding].name;
		//		if (data.find(uniformName) != data.end()) {

		//			Buffer& dataBuffer = data.at(uniformName);
		//			ub->SetData(dataBuffer, dataBuffer.GetSize(), 0);
		//		}

		//	}

		//}


		//// three frame index has the same texture, so just use the first one :(
		//for (auto [set, bindingMap] : m_Textures[0]) {

		//	for (auto [binding, texture] : bindingMap) {

		//		Ref<VulkanTexture2D> texture = std::static_pointer_cast<VulkanTexture2D>(texture);

		//		VkWriteDescriptorSet& writeDescriptor = m_WriteDescriptors[frameIndex][set][binding];
		//		writeDescriptor.pImageInfo = &texture->GetDescriptorImageInfo();
		//		vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);

		//	}

		//}


		// for now didn't support change texture in material submit

	}

	const std::unordered_map<uint32_t, VkDescriptorSet>& DescriptorSetManager::GetDescriptorSets() const
	{

		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();

		return m_DescriptorSets[frameIndex];
	}

	const std::unordered_map<uint32_t, VkDescriptorSet>& DescriptorSetManager::GetDescriptorSets(uint32_t frameIndex) const
	{

		HZ_CORE_ASSERT(m_DescriptorSets.size() >= frameIndex, "Descriptor set out of range!");

		return m_DescriptorSets[frameIndex];
	}

}



