#pragma once

#include "Hazel/Renderer/Material.h"
#include "DescriptorSetManager.h"
#include <vulkan/vulkan.h>


namespace Hazel {
	
	class Texture2D;
	class DescriptorSetManager;
	class Shader;

	class VulkanMaterial : public Material{


	public:
		
		VulkanMaterial(Ref<Shader> shader);
								
		template<typename T>
		bool SetData(const std::string& name, T& value, uint32_t index) {

			return m_DescriptorSetManager->SetData(name, value, index);					
		}
		
		bool VulkanMaterial::SetData(const std::string& name, Ref<UniformBufferSet>& bufferSet, uint32_t index) {

			return m_DescriptorSetManager->SetData(name, bufferSet, index);
		}
		
		bool VulkanMaterial::SetData(const std::string& name, const Ref<Texture2D>& texture, uint32_t index) {

			return m_DescriptorSetManager->SetData(name, texture, index);

		}
		

		const std::unordered_map<uint32_t, VkDescriptorSet>& GetDescriptorSets(uint32_t frameIndex) const;

		virtual void Submit() override;

		virtual void SetShader(Ref<Shader> shader) override;

	private:
				

		Scope<DescriptorSetManager> m_DescriptorSetManager;
		

		friend class VulkanRenderAPI;
	};


}