#pragma once


#include "glm/glm.hpp"
#include "VertexArray.h"
#include "Hazel/Renderer/Texture.h"

#include "entt.hpp"

struct aiScene;
struct aiNode;

namespace Hazel {



	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec2 texcoord;
		int entityID = -1;
	};

	struct Face {
		uint32_t v1, v2, v3;
	};

	class SubMesh {
	public:
		uint32_t baseVertex;
		uint32_t baseIndex;
		uint32_t materialIndex;
		uint32_t indexCount;

		glm::mat4 transform;
		//AABB BoundingBox;

		std::string nodeName, meshName;
	};

	enum MeshType {
		Invalid, Static, Animated
	};

	struct Triangle {
		Vertex v0, v1, v2;
	};

	class Mesh {
	public:

		friend class Renderer;

		Mesh() = default;
		Mesh(std::string path, entt::entity entityID);
		~Mesh() = default;


		std::string GetFilePath() { return m_FilePath; }
		MeshType GetMeshType() { return m_Type; }

		Ref<Texture2D> GetTexture(TextureType type);
	private:


		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
		void Submit();

		friend class Editor;


		Ref<VertexArray> m_VertexArray;
		std::vector<Vertex> m_Vertices;
		std::vector<Face> m_Faces;

		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;

		std::vector<SubMesh> m_SubMeshes;

		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

		const aiScene* m_Scene;

		std::string m_FilePath;

		MeshType m_Type = Invalid;

		bool m_IsVisiable = true;

		bool m_IsAnimated = false;

	};


	
}