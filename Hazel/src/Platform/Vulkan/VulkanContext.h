#pragma once

#include "Hazel/Renderer/GraphicsContext.h"


namespace Hazel {


	class VulkanContext  : public GraphicsContext{

	public:
		virtual void Init() override;
		virtual void Swapbuffers() override;

	};


}