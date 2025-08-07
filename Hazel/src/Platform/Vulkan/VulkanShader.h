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


	

	class VulkanShader : public Shader {

	public:

		VulkanShader(const std::string& filePath);

		VkShaderModule GetShaderModule(ShaderType type);


		// Implementation of the useless Shader interface
		virtual void				Bind			() const override {}
		virtual void				Unbind			() const override {}
		virtual const std::string	GetName			() const override { return ""; }
		virtual bool				Reload			() override { return false; }
		virtual const void			SetInt			(const std::string& name, const int val) override {}
		virtual const void			SetFloat		(const std::string& name, const float val) override {}
		virtual const void			SetFloat2		(const std::string& name, const glm::vec2& val) override {}
		virtual const void			SetFloat3		(const std::string& name, const glm::vec3& val) override {}
		virtual const void			SetFloat4		(const std::string& name, const glm::vec4& val) override {}
		virtual const void			SetMat3			(const std::string& name, const glm::mat3& val) override {}
		virtual const void			SetMat4			(const std::string& name, const glm::mat4& val) override {}
		virtual const void			SetIntArray		(const std::string& name, const int* val, const uint32_t count) override {}		
		virtual const GLint			GetRendererID	() override { return 0; }
		virtual void				Submit			(std::unordered_map<std::string, Buffer>& data) override;
		

		inline std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() { return m_DescriptorSetLayouts; };


		std::vector<std::vector<ShaderReflectionData>>& GetReflectionData() { return m_RelectionData; }
		
		ShaderReflectionData* GetReflectionDataByName(const std::string& name) {
			if (m_RelectionDataByName.find(name) != m_RelectionDataByName.end()) {
				return &m_RelectionDataByName.at(name);
			}			
			return nullptr;
		}

	private:

		void CreateShaderModules();
		bool Compile(std::vector<uint32_t>& outputBinary, ShaderType type);
		void Reflect(ShaderType shaderType, const std::vector<uint32_t>& spirvBinary);		
		void CreateDescriptorSetLayout();

	private:

		std::unordered_map<ShaderType, std::string>				m_ShaderCodeByShaderType;
		std::unordered_map<ShaderType, VkShaderModule>			m_ShaderModuleByShaderType;
		std::unordered_map<ShaderType, std::string>				m_SPIRVFilePathByShaderType;
		std::unordered_map<ShaderType, std::vector<uint32_t>>	m_SPIRVBinaryByShaderType;

		std::unordered_map<std::string, ShaderReflectionData> m_RelectionDataByName;

		// [frameIndex][set]
		std::vector<std::vector<ShaderReflectionData>> m_RelectionData;
		// [set]
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		
	};


}