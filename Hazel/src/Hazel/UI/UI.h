#pragma once
#include "imgui.h"
#include "Hazel/Renderer/Texture.h"
namespace Hazel {
	class UI {
	public:
		template<typename TFunc>
		static void HandleContentBrowserDragDrop(Ref<Texture2D>& texture, TFunc func) {
			if (ImGui::BeginDragDropTarget()) {

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					func(texture, payload->Data);
				}
				ImGui::EndDragDropTarget();
			}
		}

	};

}