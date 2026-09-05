#include "hzpch.h"

#include "Hazel/Resource/AssimpImporter.h"

namespace Hazel {




	Ref<Mesh> AssimpImporter::ImportMesh()
	{
		return Ref<Mesh>();
	}

	Ref<Animation> AssimpImporter::ImportAnimation()
	{
		return Ref<Animation>();
	}

	std::pair<Ref<Mesh>, Ref<Animation>> AssimpImporter::ImportModel(std::filesystem::path filePath)
	{
		return std::pair<Ref<Mesh>, Ref<Animation>>(ImportMesh(), ImportAnimation());
	}

}