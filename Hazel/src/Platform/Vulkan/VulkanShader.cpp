#include "hzpch.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/DescriptorSetManager.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include <stdexcept>
#include <cstdlib>
#include <fstream>
#include <windows.h>
#include <filesystem>
#include <string>
#include <codecvt>

#include <spirv_cross/spirv.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <shaderc/shaderc.hpp>

namespace Hazel {


	namespace Utils {

		std::string g_TempfilePath = "assets/Shaders/";
		std::string g_OutputSPIRVFilePath = "assets/Shaders/";

		std::string ReadShaderFile(const std::string& filepath)
		{
			std::string result;
			std::ifstream in(filepath, std::ios::in, std::ios::binary);
			if (in) {
				in.seekg(0, std::ios::end);
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());
				in.close();
			}
			else {
				HZ_CORE_ERROR("Could not open file '{0}'", filepath);
			}

			result = result.substr(0, result.find_first_of('\0'));
			return result;
		}

		std::string ReadSPIRVFile(const std::string& filePath) {

			std::ifstream file(filePath, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file");
			}
			size_t fileSize = (size_t)file.tellg();
			std::string buffer;
			buffer.resize(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		// return spriv byte shader code
		void CompileShaderFileToSpriv(const std::string& inputFilePath, const std::string& outputFilePath) {

			std::string command = "glslc.exe";
			command += ' ' + inputFilePath;
			command += " -o ";
			command += outputFilePath;

			HZ_CORE_INFO("Compiling Shader {}", inputFilePath);


			try {
				DWORD exitCode;
				std::wstring output;

				// 安全属性结构，允许句柄继承
				SECURITY_ATTRIBUTES saAttr;
				saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
				saAttr.bInheritHandle = TRUE;
				saAttr.lpSecurityDescriptor = NULL;

				// 创建匿名管道用于捕获标准输出
				HANDLE hChildStd_OUT_Rd = NULL;
				HANDLE hChildStd_OUT_Wr = NULL;
				if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
					std::wcerr << L"创建管道失败，错误码: " << GetLastError() << std::endl;
				}

				// 设置句柄为不可继承
				SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

				// 配置进程启动信息
				STARTUPINFOW si;
				PROCESS_INFORMATION pi;
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				si.hStdError = hChildStd_OUT_Wr;
				si.hStdOutput = hChildStd_OUT_Wr;
				si.dwFlags |= STARTF_USESTDHANDLES;

				ZeroMemory(&pi, sizeof(pi));

				// 安全处理命令行字符串
				std::wstring wideCommand = std::wstring(command.begin(), command.end());
				std::vector<wchar_t> cmdLine(wideCommand.begin(), wideCommand.end());
				cmdLine.push_back(L'\0'); // 确保以NULL结尾

				// 创建子进程
				BOOL bSuccess = CreateProcessW(
					NULL,                   // 不指定可执行文件
					cmdLine.data(),         // 使用可修改的缓冲区
					NULL,                   // 进程安全属性
					NULL,                   // 线程安全属性
					TRUE,                   // 句柄可继承
					CREATE_NO_WINDOW,       // 创建无窗口进程
					NULL,                   // 使用父进程环境
					NULL,                   // 使用父进程目录
					&si,                    // 启动信息
					&pi                     // 进程信息
				);

				// 关闭不需要的管道写入端
				CloseHandle(hChildStd_OUT_Wr);

				if (!bSuccess) {
					HZ_CORE_ERROR("Failed to create process, error code: {}", GetLastError());
					CloseHandle(hChildStd_OUT_Rd);
				}

				// 读取子进程输出
				const int BUFFER_SIZE = 4096;
				std::vector<char> buffer(BUFFER_SIZE); // 使用char数组而非wchar_t
				DWORD bytesRead;
				bool hasOutput = false;

				while (true) {
					// 检查进程是否已退出
					if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
						// 进程已退出，读取剩余输出
						if (PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0) {
							if (ReadFile(hChildStd_OUT_Rd, buffer.data(), BUFFER_SIZE - 1, &bytesRead, NULL)) {
								buffer[bytesRead] = '\0'; // 正确设置字符串结束符
								// 转换为宽字符串
								std::wstring wideOutput = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(buffer.data());
								output += wideOutput;
								hasOutput = true;
							}
						}
						break;
					}

					// 读取管道中的数据
					if (PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0) {
						if (ReadFile(hChildStd_OUT_Rd, buffer.data(), BUFFER_SIZE - 1, &bytesRead, NULL)) {
							buffer[bytesRead] = '\0'; // 正确设置字符串结束符
							// 转换为宽字符串
							std::wstring wideOutput = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(buffer.data());
							output += wideOutput;
							hasOutput = true;
						}
					}

					// 避免CPU占用过高
					Sleep(10);
				}

				// 获取进程退出码
				if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
					std::wcout << L"进程退出码: " << exitCode << std::endl;
				}
				else {
					std::wcerr << L"获取进程退出码失败，错误码: " << GetLastError() << std::endl;
					exitCode = 1;
				}

				// 清理资源
				CloseHandle(hChildStd_OUT_Rd);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				// 根据退出码判断是否成功
				if (exitCode == 0) {
					HZ_CORE_INFO("Compiling Shader {} Success. Save temp file to {}", inputFilePath, outputFilePath);
				}
				else {
					HZ_CORE_ERROR("Compiling Shader {} Failed. Exit code: {}", inputFilePath, exitCode);
					if (!output.empty()) {
						// 转换为窄字符串并输出错误信息
						std::string errorMsg = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(output);
						HZ_CORE_ERROR("Compiler Output: {}", errorMsg);
					}
				}
			}
			catch (const std::exception& e) {
				HZ_CORE_ERROR("Exception during shader compilation: {}", e.what());
			}
		}

		void CompileShaderCodeToSpriv(std::string& code, const std::string& outputFilePath, ShaderType type) {

			//Create temporary file		


			uint64_t uuid = UUID();

			std::string tempfilePath = g_TempfilePath + std::to_string(uuid);

			switch (type)
			{

			case ShaderType::VertexShader:
				tempfilePath += ".vert";
				break;
			case ShaderType::FragmentShader:
				tempfilePath += ".frag";
				break;
			case ShaderType::None:
			default:
				HZ_CORE_ASSERT(false, "Invaild shader type specified!");
				break;
			}


			std::ofstream tempfile(tempfilePath, std::iostream::binary);
			if (!tempfile.is_open()) {
				HZ_CORE_ASSERT(false, "Can't create temp file in {}!", tempfilePath);
			}

			code = code.substr(0, code.find_first_of('\0'));

			tempfile << code << "\r\n";
			tempfile.close();


			CompileShaderFileToSpriv(tempfilePath, outputFilePath);



			std::filesystem::remove(tempfilePath);

		}



		VkDescriptorType GetVulkanDescriptorTypeFromDescriptorType(DescriptorType type)
		{
			switch (type) {
			case DescriptorType::UniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType::StorageBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorType::Sampler2D:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
		}

		VkShaderStageFlagBits GetVulkanShaderStageFlagBitsFromShaderType(ShaderType type)
		{
			switch (type) {
			case ShaderType::VertexShader:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::FragmentShader:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case ShaderType::ComputeShader:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}
		}

		shaderc_shader_kind GetShaderCKindFormShaderType(ShaderType type)
		{
			switch (type) {
			case ShaderType::VertexShader:
				return shaderc_vertex_shader;
			case ShaderType::FragmentShader:
				return shaderc_fragment_shader;
			case ShaderType::ComputeShader:
				return shaderc_compute_shader;
			case ShaderType::None:
				HZ_CORE_ASSERT(false, "Invalid shader type!");
			}
		}

		std::unordered_map<ShaderType, std::string> PreprocessShaderFile(std::string& src)
		{
			std::unordered_map<ShaderType, std::string> result;

			const char* typeToken = "#type";
			size_t typeTokenLength = strlen(typeToken);
			size_t pos = src.find(typeToken, 0);

			while (pos != std::string::npos) {
				size_t eol = src.find_first_of("\r\n", pos);
				HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
				size_t begin = pos + typeTokenLength + 1;
				std::string type = src.substr(begin, eol - begin);
				HZ_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invaild shader type specified!");

				/*if (type == "vertex" || type == "fragment" || type == "pixel") {
					m_Type = ShaderType::VertAndFragShader;
				}
				else if (type == "compute") {
					m_Type = ShaderType::ComputeShader;
				}*/

				size_t nextLinePos = src.find_first_not_of("\r\n", eol);
				pos = src.find(typeToken, nextLinePos);
				result[GetShaderTypeFromString(type)] =
					src.substr(nextLinePos,
						pos - (nextLinePos == std::string::npos ? src.size() - 1 : nextLinePos));

				std::string& sourceCode = result[GetShaderTypeFromString(type)];


				//CompileThe


			}

			return result;
		}


		VkShaderModule CreateShaderModuleFromFile(const std::string& filePath, VkDevice logicalDevice) {


			std::ifstream spirvFile(filePath, std::ios::ate | std::ios::binary);

			if (!spirvFile.is_open()) {

				HZ_CORE_ASSERT(false, "Can't not open SPIRV file : {}", filePath);
				return nullptr;
			}
			size_t fileSize = (size_t)spirvFile.tellg();
			std::string buffer;
			buffer.resize(fileSize);

			spirvFile.seekg(0);
			spirvFile.read(buffer.data(), fileSize);

			spirvFile.close();

			std::vector<uint32_t> binary = { buffer.begin(), buffer.end() };

			return CreateShaderModuleFromBinary(binary, logicalDevice);

		}

		VkShaderModule CreateShaderModuleFromBinary(const std::vector<uint32_t>& binary, VkDevice logicalDevice) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = binary.size() * sizeof(uint32_t);
			createInfo.pCode = binary.data();

			VkShaderModule shaderModule;

			HZ_CORE_ASSERT(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "failed to create shader Module");

			return shaderModule;
		}

		ShaderType GetShaderTypeFromString(const std::string& type) {

			if (type == "vertex") {
				return ShaderType::VertexShader;
			}
			else if (type == "fragment" || type == "pixel") {
				return ShaderType::FragmentShader;
			}
			else if (type == "compute") {
				return ShaderType::ComputeShader;
			}
			HZ_CORE_ASSERT(false, "Invaild shader type specified!");
		}

	}


	VulkanShader::VulkanShader(const std::string& filePath) {


		m_Path = filePath;
		m_Name = Utils::GetShaderName(filePath);

		int lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash;
		int lastDot = filePath.find_last_of('.');
		int count = lastDot == std::string::npos ? filePath.size() - lastSlash - 1 : lastDot - lastSlash - 1;
		m_Name = filePath.substr(lastSlash + 1, count);
		
		Reload();		
	}


	VkShaderModule VulkanShader::GetShaderModule(ShaderType type) {

		auto it = m_ShaderModules.find(type);
		if (it != m_ShaderModules.end())
			return it->second;

		HZ_CORE_ASSERT(false, "This Vulkan Shader don't have fit Shader Module!");

	}

	bool VulkanShader::Reload()
	{
		m_ShaderCodes.clear();

		std::string shaderSource = Utils::ReadShaderFile(m_Path);
		m_ShaderCodes = Utils::PreprocessShaderFile(shaderSource);

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		for(auto [ShaderType, shaderModule] : m_ShaderModules)
		{			
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}

		m_ShaderModules.clear();
		m_RelectionData.clear();
		
		m_DescriptorSetLayouts.clear();
		m_SPIRVBinarys.clear();

		for (auto& [shaderType, code] : m_ShaderCodes) {
			m_SPIRVBinarys.emplace(shaderType, std::vector<uint32_t>());
			Compile(m_SPIRVBinarys.at(shaderType), shaderType);
		}

		CreateShaderModules();

		for (const auto& [shaderType, binary] : m_SPIRVBinarys) {

			if (binary.size() == 0) {
				HZ_CORE_ASSERT(false, "SPIRV Binary is empty for shader type {}", Utils::GetShaderCKindFormShaderType(shaderType));
			}
			Reflect(shaderType, binary);
		}

		CreateDescriptorSetLayout();

		return false;
		
	}

	void VulkanShader::Submit(std::unordered_map<std::string, Buffer>& data)
	{		

		

	}


	void VulkanShader::CreateShaderModules() {


		/*for (const auto& it : shaderCodeByShaderType) {



		}*/

		//TODO : make the code more robust

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		for (auto& [shaderType, binary] : m_SPIRVBinarys) {

			m_ShaderModules.emplace(shaderType, Utils::CreateShaderModuleFromBinary(binary, device));

		}

	}

	bool VulkanShader::Compile(std::vector<uint32_t>& outputBinary, ShaderType type)
	{

		std::string sourceCode = m_ShaderCodes.at(type);

		static shaderc::Compiler compiler;
		shaderc::CompileOptions shaderCOptions;
		shaderCOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		shaderCOptions.SetWarningsAsErrors();
		shaderCOptions.SetOptimizationLevel(shaderc_optimization_level_zero);

		const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(sourceCode, Utils::GetShaderCKindFormShaderType(type), m_Path.c_str());

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			HZ_CORE_WARN("{} while compiling shader file : {}", module.GetErrorMessage(), m_Path);
			return false;
		}

		outputBinary = std::vector<uint32_t>(module.begin(), module.end());
		return true;

	}

	void VulkanShader::Reflect(ShaderType stage, const std::vector<uint32_t>& spirvBinary)
	{

		spirv_cross::Compiler compiler(spirvBinary);
		auto resources = compiler.get_shader_resources();
		// uniform buffers
		for (const auto& resource : resources.uniform_buffers) {

			auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
			if (activeBuffers.size()) {

				const auto& name = resource.name;
				auto& baseType = compiler.get_type(resource.base_type_id);
				auto& type = compiler.get_type(resource.type_id);
				int memberCount = baseType.member_types.size();
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t size = (uint32_t)compiler.get_declared_struct_size(baseType);
				uint32_t arraySize = type.array.size() > 0 ? type.array[0] : 1;
				
				ShaderReflectionData reflectionData;
				reflectionData.name = name;
				reflectionData.binding = binding;
				reflectionData.descriptorSet = descriptorSet;
				reflectionData.size = size;
				reflectionData.stage = stage;
				reflectionData.type = DescriptorType::UniformBuffer;
				reflectionData.arraySize = arraySize;
				reflectionData.dimension = 1;
				
				if (m_RelectionData.find(descriptorSet) == m_RelectionData.end()) {
					m_RelectionData[descriptorSet] = std::unordered_map<uint32_t, ShaderReflectionData>();
				}				

				m_RelectionData[descriptorSet][binding] = reflectionData;
				m_RelectionDataByName[name] = reflectionData;

			}

		}
		// sampling images
		for (const auto& resource : resources.sampled_images) {

			const auto& name = resource.name;
			auto& baseType = compiler.get_type(resource.base_type_id);
			auto& type = compiler.get_type(resource.type_id);
			int memberCount = baseType.member_types.size();
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);						
			uint32_t arraySize = type.array.size() > 0 ? type.array[0] : 1;

			ShaderReflectionData reflectionData;
			reflectionData.name = name;
			reflectionData.binding = binding;
			reflectionData.size = 8; // For raw pointer			
			reflectionData.descriptorSet = descriptorSet;			
			reflectionData.stage = stage;
			reflectionData.dimension = baseType.image.dim + 1;
			reflectionData.arraySize = arraySize;
			switch (reflectionData.dimension)
			{
			case 1:
				reflectionData.type = DescriptorType::Sampler2D;
				break;
			case 2:
				reflectionData.type = DescriptorType::Sampler2D;
				break;
			case 3:
				reflectionData.type = DescriptorType::Sampler3D;
				break;
			}

			if (m_RelectionData.find(descriptorSet) == m_RelectionData.end()) {
				m_RelectionData[descriptorSet] = std::unordered_map<uint32_t, ShaderReflectionData>();
			}

			m_RelectionData[descriptorSet][binding] = reflectionData;
			m_RelectionDataByName[name] = reflectionData;

		}
	}

	void VulkanShader::CreateDescriptorSetLayout()
	{
		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		m_DescriptorSetLayouts.resize(m_RelectionData.size());

		for (uint32_t set = 0; set < m_RelectionData.size(); set++) {


			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			layoutBindings.resize(m_RelectionData[set].size());

			for (uint32_t binding = 0; binding < m_RelectionData[set].size(); binding++) {

				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings[binding];
				const ShaderReflectionData& reflectData = m_RelectionData[set][binding];
				layoutBinding.binding = reflectData.binding;
				layoutBinding.descriptorType = Utils::GetVulkanDescriptorTypeFromDescriptorType(reflectData.type);
				layoutBinding.descriptorCount = reflectData.arraySize;
				layoutBinding.stageFlags = Utils::GetVulkanShaderStageFlagBitsFromShaderType(reflectData.stage);
				layoutBinding.pImmutableSamplers = nullptr; // Optional, used for sampler bindings
			}


			VkDescriptorSetLayoutCreateInfo layoutInfo{};

			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = layoutBindings.size();
			layoutInfo.pBindings = layoutBindings.data();


			HZ_CORE_ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayouts[set]) == VK_SUCCESS, "Failed to create descriptor!");


		}

	}

}

