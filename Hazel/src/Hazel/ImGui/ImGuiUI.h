#pragma once


#include "Hazel/Renderer/Texture.h"


#include <imgui.h>

#include "Hazel/Core/Core.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

using namespace Hazel;

namespace ImGui{
	


	void Image(Ref<Texture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1);	
	void Image(Ref<OpenGLTexture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1);
	void Image(Ref<VulkanTexture2D> texture, const ImVec2& textureSize, const ImVec2& uv0, const ImVec2& uv1);


	bool ImageButton(const char* str_id, Ref<Texture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding = -1, const ImVec4& bgCol = ImVec4(0, 0, 0, 0), const ImVec4& tintCol = ImVec4(1, 1, 1, 1));
	bool ImageButton(const char* str_id, Ref<OpenGLTexture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding = -1, const ImVec4& bgCol = ImVec4(0, 0, 0, 0), const ImVec4& tintCol = ImVec4(1, 1, 1, 1));
	bool ImageButton(const char* str_id, Ref<VulkanTexture2D> texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int framePadding = -1, const ImVec4& bgCol = ImVec4(0, 0, 0, 0), const ImVec4& tintCol = ImVec4(1, 1, 1, 1));
	  
}