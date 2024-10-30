#pragma once
#include "hzpch.h"
#include "Hazel/Core.h"
#include "Hazel/Event/Event.h"

#include "Hazel/Core/Timestep.h"

namespace Hazel {

	class HAZEL_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() = 0;


		virtual void Begin();
		virtual void End();

		inline const std::string& GetName() const { return  m_DebugName; }
	protected:
		std::string m_DebugName;

	};

}