#include "hzpch.h"

#include "Hazel/Renderer/RenderDevice.h"
#include "Hazel/Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanDevice.h"

namespace Hazel {

	void RenderDevice::SetCurrent(RenderDevice* device)
	{
		s_Current = device;
	}

	RenderDevice* RenderDevice::Create() {

		switch (Renderer::GetAPI()) {
		case RendererAPI::API::Vulkan:
			return new VulkanDevice();
		case RendererAPI::API::OpenGL:
			return nullptr;
		}

	}

	RenderDevice& RenderDevice::Get()
	{
		HZ_CORE_ASSERT(s_Current, "RenderDevice current device has not been set");
		return *s_Current;
	}


	bool RenderDevice::Init() {

		m_UniformCache = MakeRef<UniformCache>();

		return true;
	}
	

}
