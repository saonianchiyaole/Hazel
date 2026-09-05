#pragma once

#include "Hazel/Renderer/Shader.h"

#include "vulkan/vulkan.h"

#include <shaderc/shaderc.hpp>

namespace Hazel {


	class VulkanUniformBuffer;
	class DescriptorSetManager;
	class CommandBuffer;
	
	struct DescriptorSetLayout {

	};


	namespace Utils {

		std::string									ReadShaderFile(const std::string& filePath);

		VkShaderModule								CreateShaderModuleFromBinary(const std::vector<uint32_t>& binary, VkDevice logicalDevice);

		VkShaderModule								CreateShaderModuleFromFile(const std::string& filePath, VkDevice logicalDevice);

		std::unordered_map<ShaderType, std::string> PreprocessShaderFile(std::string& src);

		ShaderType									GetShaderTypeFromString(const std::string& type);

		// input shader text code and return spirv byte shader code
		void										CompileShaderFileToSpriv(const std::string& inputFilePath, const std::string& outputFilePath);

		void										CompileShaderCodeToSpriv(std::string& code, const std::string& outputFilePath, ShaderType type);

		VkDescriptorType							GetVulkanDescriptorTypeFromDescriptorType(DescriptorType type);

		VkShaderStageFlagBits						GetVulkanShaderStageFlagBitsFromShaderType(ShaderType type);

		shaderc_shader_kind							GetShaderCKindFormShaderType(ShaderType type);

	}


	

	class VulkanShader{

	public:

		VulkanShader(Ref<const ShaderSnapshot> snapShot);

		VkShaderModule GetShaderModule(ShaderType type);
			
		bool Reload();

		inline std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() { return m_DescriptorSetLayouts; };
		
		Ref<const ShaderSnapshot> GetSnapshot() const { return m_SnapShot; }

	private:

		void CreateShaderModules();		
		void CreateDescriptorSetLayout();

	private:

		Ref<const ShaderSnapshot> 								m_SnapShot;				
				
		std::unordered_map<ShaderType, VkShaderModule>			m_ShaderModules;		
			
		// [set]
		std::vector<VkDescriptorSetLayout>						m_DescriptorSetLayouts;
		
	};


}
