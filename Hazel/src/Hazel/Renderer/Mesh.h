#pragma once




#include "glm/glm.hpp"
#include "VertexArray.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Core/Timestep.h"


#include "entt.hpp"

#include "assimp/matrix4x4.h"
#include "assimp/Importer.hpp"
#include "Hazel/Asset/Asset.h"

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Hazel {

	class Material;

	namespace Utils {
		std::string TextureTypeToString(TextureType type);

		glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix);
	}


	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec2 texcoord;
		glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	};

	struct BoneInfo
	{
		glm::mat4 boneOffset;
		glm::mat4 finalTransformation;
		uint32_t id;
	};

	struct BoneInfluence
	{
		uint32_t IDs[4] = {0, 0, 0, 0};
		float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};

		BoneInfluence() = default;

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			if (Weight <= 0.0f)
				return;
			for (size_t i = 0; i < 4; i++)
			{
				if (weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					weights[i] = Weight;
					return;
				}
			}

			// should never get here - more bones than we have space for
			HZ_CORE_WARN("Vertex has more than four bones / weights affecting it, extra data will be discarded(BoneID = { 0 }, Weight = { 1 })", BoneID, Weight);
		}
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

		Ref<VertexArray> vertexArray = nullptr;
		std::vector<Vertex> vertices;
		std::vector<BoneInfluence> boneInfluences;
		std::vector<Face> faces;

		bool isRigged = false;

		glm::mat4 transform = glm::mat4(1.0f);
		//AABB BoundingBox;

		std::string nodeName, meshName;

	};


	struct Triangle {
		Vertex v0, v1, v2;
	};

	class Mesh : public Asset{
	public:
		friend class Animation;
		friend class Renderer;

		Mesh() = default;
		Mesh(const std::string& path, entt::entity entityID);
		~Mesh() = default;

		std::string GetFilePath() { return m_FilePath; }

		virtual AssetType GetAssetType() override{
			return AssetType::Mesh;
		}

		bool IsAnimated() { return m_IsAnimated; }

		Ref<Texture2D> GetTexture(TextureType type);

		uint32_t GetBoneAmount() { return m_BoneCount; }
		const glm::mat4& GetBoneTransform(uint32_t index) const { return m_BoneTransforms[index]; }
		std::vector<Ref<SubMesh>> GetSubMeshes() { return m_SubMeshes; }
		std::vector<Ref<Material>> GetMaterials() { return m_Materials; }

	private:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
		void ExtractBones();

		friend class Editor;

		//Ref<VertexArray> m_VertexArray;
		std::vector<Vertex> m_Vertices;
		std::vector<Face> m_Faces;
		std::vector<BoneInfluence> m_BoneInfluences;

		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;

		std::vector<Ref<Material>> m_Materials;

		glm::mat4 m_InverseTransform;

		std::vector<Ref<SubMesh>> m_SubMeshes;

		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;
		std::vector<glm::mat4> m_BoneTransforms;

		Scope<Assimp::Importer> m_Importer;
		const aiScene* m_Scene;

		std::unordered_map<std::string, uint32_t> m_BoneNameToIndex;
		uint32_t m_BoneCount;
		std::vector<BoneInfo> m_BoneInfo;

		std::string m_FilePath;

		bool m_IsVisiable = true;
		
		bool m_IsAnimated = false;

	};


	
}