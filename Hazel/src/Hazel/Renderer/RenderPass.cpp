#include "hzpch.h"
#include "RenderPass.h"

namespace Hazel {



	RenderPass::RenderPass(RenderPassSpec spec)
		: m_Specification(spec)
	{
	}

	RenderPassSpec Hazel::RenderPass::GetSpecification()
	{
		return m_Specification;
	}

	Ref<RenderPass> RenderPass::Create(RenderPassSpec spec)
	{
		Ref<RenderPass> renderPass = MakeRef<RenderPass>(spec);
		return renderPass;
	}

}