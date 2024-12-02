#include "hzpch.h"
#include "Panels/ContentBrowserPanel.h"
#include <imgui.h>

namespace Hazel {

	ContentBrowserPanel::ContentBrowserPanel(std::string assetsDirectory)
		: m_AssetsDirectory(assetsDirectory)
	{
		m_CurrentDirectory = m_AssetsDirectory;
		m_FolderTexture = Texture2D::Create("assets/ContentBrowserIcon/folderIcon.png");
		m_FileTexture = Texture2D::Create("assets/ContentBrowserIcon/fileIcon.png");

	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("ContentBrowser");


		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;


		bool isBackToParentPath = false;
		ImGui::BeginDisabled(m_CurrentDirectory == m_AssetsDirectory);

		if (ImGui::Button("<-")) {
			isBackToParentPath = true;
		}

		ImGui::EndDisabled();
		
		ImGui::SameLine();
		if(isBackToParentPath)
			m_CurrentDirectory = m_CurrentDirectory.parent_path();

		char buffer[1000];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), m_CurrentDirectory.string().c_str());
		std::filesystem::path inputDirectory;
		if (ImGui::InputText("##DirectoryBuffer", buffer, sizeof(buffer))) {
			if (std::filesystem::exists(buffer)) {
				m_CurrentDirectory = buffer;
			}
		}

		ImGui::Columns(1);


		ImGui::Columns(columnCount, 0, false);

		for (auto& directotyEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {

			const auto& path = directotyEntry.path().filename().string();
			bool isFolder = directotyEntry.is_directory() ? true : false;
			Ref<Texture2D> icon = isFolder ? m_FolderTexture : m_FileTexture;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(path.c_str(), (ImTextureID)icon->GetRendererID(), { 30, 30 }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource()) {

				std::filesystem::path targetPath(m_CurrentDirectory / path);
				const wchar_t* itemPath = targetPath.c_str();

				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

				ImGui::EndDragDropSource();
			}


			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

				if (isFolder) {
					m_CurrentDirectory.append(path);
				}
			}

			ImGui::TextWrapped(path.c_str());
			ImGui::NextColumn();

			ImGui::PopStyleColor();
		}

		ImGui::Columns(1);


		ImGui::End();
	}




}