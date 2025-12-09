#include "hzpch.h"
#include "Hazel/ImGui/ImGuiUI.h"
#include "Hazel/Renderer/RendererAPI.h"

#include "backends/imgui_impl_vulkan.cpp"
#include <imgui_internal.h>

namespace ImGui {



	void Image(Ref<Texture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1)
	{
		
		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL: 
		{
			Ref<OpenGLTexture2D> openGLTexture = std::static_pointer_cast<OpenGLTexture2D>(texture);
			Image(openGLTexture, textureSize, uv0, uv1);
			break;
		}
		case RendererAPI::API::Vulkan: {
			Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
			Image(vulkanTexture, textureSize, uv0, uv1);
			break;
		}
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid render API!");
		}
	}


	void Image(Ref<OpenGLTexture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1) {

		ImGui::Image((void*)texture->GetRendererID(), {(float)textureSize.x, (float)textureSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
	}
	void Image(Ref<VulkanTexture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1) {
		
		Utils::TransitionImageLayout(texture->GetRawImage(), Utils::GetVulkanFormatFromTextureFormat(texture->GetTextureFormat()), texture->GetLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		void* descriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImGui::Image(descriptorSet, textureSize, uv0, uv1);
	}



	bool ImageButton(const char* str_id, Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding, const ImVec4& bgCol, const ImVec4& tintCol)
	{
		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			Ref<OpenGLTexture2D> openGLTexture = std::static_pointer_cast<OpenGLTexture2D>(texture);
			return ImageButton(str_id, openGLTexture, size, uv0, uv1);
			break;
		}
		case RendererAPI::API::Vulkan: {
			Ref<VulkanTexture2D> vulkanTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
			return ImageButton(str_id, vulkanTexture, size, uv0, uv1);
			break;
		}
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "Invalid render API!");
		}
	}

	bool ImageButton(const char* str_id, Ref<OpenGLTexture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding, const ImVec4& bgCol, const ImVec4& tintCol)
	{
		ImGuiID id = ImGui::GetID(str_id);
		return ImGui::ImageButtonEx(id, (ImTextureID)texture->GetRendererID(), size, uv0, uv1, bgCol, tintCol);
	}

	bool ImageButton(const char* str_id, Ref<VulkanTexture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding, const ImVec4& bgCol, const ImVec4& tintCol)
	{
		ImGuiID id = ImGui::GetID(str_id);

		void* descriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return ImGui::ImageButtonEx(id, (ImTextureID)descriptorSet, size, uv0, uv1, bgCol, tintCol);
	}


}