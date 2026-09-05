#pragma once



namespace Hazel {
	
	class Mesh;
	class Animation;
	class AssimpImporter {

	public:
		static std::pair<Ref<Mesh>, Ref<Animation>> ImportModel(std::filesystem::path filePath);
	
	private:
		static Ref<Mesh> ImportMesh();
		static Ref<Animation> ImportAnimation();
	};

}