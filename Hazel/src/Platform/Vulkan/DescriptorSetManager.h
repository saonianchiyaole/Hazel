#pragma once


#include "Platform/Vulkan/Vulkan.h"
#include "Hazel/Core/Buffer.h"


#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/RenderDevice.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Hazel {

	class VulkanUniformBuffer;
	class VulkanTexture2D;
	class CommandBuffer;
	class VulkanShader;
	class Texture;
	class Texture2D;
	class Shader;
	class UniformBufferSet;
	class UniformBuffer;
	class RenderDevice;
	class VulkanDevice;

	
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

	struct DescriptorSetManagerInfo {

		VulkanDevice* device = nullptr;
	
	};

	
	class DescriptorSetManager
	{

	public:

		DescriptorSetManager() = default;
		DescriptorSetManager(const DescriptorSetManagerInfo& specification);

		~DescriptorSetManager();

		static Scope<DescriptorSetManager> Create(const DescriptorSetManagerInfo& spec);

		void Init();								
		void Prepare(Handle<Shader> shader);
				
		const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets() const;
		std::vector<VkDescriptorSet> GetSortedDescriptorSets();


		template<typename T>
		bool SetData(const std::string& name, const T value, uint32_t index = 0) {
			Ref<VulkanShader> shader = GetShaderProxy();
			HZ_CORE_ASSERT(shader, "Invalid Vulkan shader handle in DescriptorSetManager");

			ShaderReflectionData* rd = shader->GetReflectionDataByName(name);

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
			
			m_UniformBuffers[set][binding][index]->SetData((void*)&value, sizeof(T), 0);

			return true;
		}
						
		virtual bool SetData(const std::string& name, Ref<UniformBuffer> uniformBuffer, uint32_t index = 0);
		virtual bool SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0);
		virtual bool SetData(Ref<UniformCache> uniformCache);

		inline VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }


		static std::vector<VkDescriptorSet> GetSortedDescriptorSets(const std::unordered_map<uint32_t, VkDescriptorSet>& descriptorSets);

	protected:
				
		
		DescriptorSetManagerInfo m_Info;

		VkDescriptorPool m_DescriptorPool;		

		// set -> descriptor set
		std::unordered_map<uint32_t, VkDescriptorSet> m_DescriptorSets;
						
		// set -> binding -> uniform buffer array
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<Ref<VulkanUniformBuffer>>>> m_UniformBuffers;
		
		// set -> binding -> imageBuffer
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<Ref<VulkanTexture2D>>>> m_Textures;

		// set -> binding -> VkWriteDescriptorSet
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkWriteDescriptorSet>> m_WriteDescriptors;		

		Handle<Shader> m_BakedShader;
		
	};



}
