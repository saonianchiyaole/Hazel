#pragma once


#include "Platform/Vulkan/Vulkan.h"
#include "Hazel/Core/Buffer.h"


#include "Hazel/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Hazel {

	class VulkanUniformBuffer;
	class CommandBuffer;
	class VulkanShader;
	class Texture;
	class Texture2D;
	class Shader;
	class UniformBufferSet;
	class UniformBuffer;

	typedef std::unordered_map<std::string, Buffer> MaterialDataMap;


	struct DescriptorSetSpecification {
		
		VkDescriptorSetLayoutBinding binding;
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
		
		uint32_t descriptorCount = 1;
	
	};

	struct DescriptorSpecification {

		uint32_t binding;		

	};	

	enum class DescriptorSetManagerUsage {
		RenderPass, Material
	};

	struct DescriptorSetManagerSpecification {

		Ref<Shader> shader;

		DescriptorSetManagerUsage usage;
	};

	
	class DescriptorSetManager
	{

	public:

		DescriptorSetManager() = default;
		DescriptorSetManager(const DescriptorSetManagerSpecification& specification);

		virtual ~DescriptorSetManager();

		static Scope<DescriptorSetManager> Create(const DescriptorSetManagerSpecification& spec);

		virtual void Init();								
		
		virtual void Submit();

		virtual void Submit(MaterialDataMap& data);

		virtual const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets() const;
		virtual const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets(uint32_t frameIndex) const;


		template<typename T>
		bool SetData(const std::string& name, T& value, uint32_t index = 0) {

			Ref<VulkanShader> shader = std::static_pointer_cast<VulkanShader>(m_Specification.shader);

			ShaderReflectionData* rd = shader->GetReflectionDataByName(name);
			uint32_t frameInFlight = Renderer::GetFrameInFlight();
			uint32_t frameIndex = Renderer::GetCurrentFrameIndex();


			if (!rd) {
				HZ_CORE_ASSERT(false, "Can't find uniform named : {0}", name);
				return false;
			}

			if (rd->size != sizeof(T)) {
				HZ_CORE_ASSERT(false, "Data size should equal uniform size : {0}", name);
				return false;
			}

			uint32_t set = rd->descriptorSet;
			uint32_t binding = rd->binding;
			

			for (uint32_t i = 0; i < frameInFlight; i++) {
				m_UniformBuffers[i][set][binding][index]->SetData((void*)&value, sizeof(T), 0);
			}

			return true;
		}

				//bool SetData(const std::string& name, Ref<UniformBufferSet>& uniformBufferSet, int32_t index = 0);
		virtual bool SetData(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0);
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0);
		virtual bool SetData(const std::string& name, Ref<Texture2D> texture, uint32_t index = 0);
	

		inline VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }


		static std::vector<VkDescriptorSet> GetSortedDescriptorSets(const std::unordered_map<uint32_t, VkDescriptorSet>& descriptorSets);

	protected:
		
		

		DescriptorSetManagerSpecification m_Specification;

		VkDescriptorPool m_DescriptorPool;		

		//[frame] set
		std::vector<std::unordered_map<uint32_t, VkDescriptorSet>> m_DescriptorSets;
				
		
		// [frame] set -> binding -> uniformBufer
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<Ref<UniformBuffer>>>>> m_UniformBuffers;
		
		// set -> binding -> imageBuffer
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<Ref<Texture>>>> m_Textures;

		// [frame] set -> binding -> VkWriteDescriptorSet
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkWriteDescriptorSet>>> m_WriteDescriptors;		

	};



}