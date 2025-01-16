#include <filesystem>
#include <imgui.h>
#include "Graphics.h"
#include "ResourceManager.h"

// ���f�����\�[�X�ǂݍ���
std::shared_ptr<ModelResource> ResourceManager::LoadModelResource(const char* filename)
{
	return nullptr;
}

// �f�o�b�OGUI�`��
void ResourceManager::DrawDebugGUI()
{
	if (ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto it = models.begin(); it != models.end(); ++it)
		{
			std::filesystem::path filepath(it->first);

			int use_count = it->second.use_count();
			ImGui::Text("use_count = %5d : %s", use_count, filepath.filename().u8string().c_str());
		}
	}
}

