#include "hzpch.h"
#include "Hazel/Renderer/RenderCommand.h"

#include "Hazel/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Hazel {
	
	Scope<RendererAPI> RenderCommand::s_RendererAPI = nullptr;

}