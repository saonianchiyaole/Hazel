#pragma once
#include "ContentBrowserPanel.h"
#include <filesystem>
#include "Hazel/Renderer/Texture.h"


namespace Hazel {



	class ContentBrowserPanel {

	public:
		ContentBrowserPanel() = default;
		ContentBrowserPanel(std::string assetsPath);

		void OnImGuiRender();

	private:
		std::filesystem::path m_AssetsDirectory;
		std::filesystem::path m_CurrentDirectory;
		Ref<Texture2D> m_FolderTexture;
		Ref<Texture2D> m_FileTexture;

	};


}