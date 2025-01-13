#include "hzpch.h"

#include "Hazel/Renderer/Mesh.h"


#include "assimp/scene.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

namespace Hazel {

	namespace Utils {
		std::string TextureTypeToString(TextureType type) {
			switch (type)
			{
			case TextureType::None:
				return "None";
			case TextureType::Albedo:
				return "Albedo";
			case TextureType::Roughness:
				return "Roughness";
			case TextureType::Metalness:
				return "Metalness";
			case TextureType::Normal:
				return "Normal";
			default:
				break;
			}
		}
	}


	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ValidateDataStructure;    // Validation

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			HZ_CORE_ERROR("Assimp error: {0}", message);
		}
	};


	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
		result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
		result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
		result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
		return result;
	}

	Mesh::Mesh(std::string path, entt::entity entityID)
		:m_FilePath(path)
	{

		HZ_CORE_INFO("Open Scene filePath {0}", path);
		if (path.empty()) {
			HZ_CORE_WARN("This is Not Scene file!");
			return;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
			HZ_CORE_ERROR("Failed to load mesh fild: {0}", path);

		m_Scene = scene;



		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		m_SubMeshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; m++) {
			aiMesh* mesh = scene->mMeshes[m];

			SubMesh& subMesh = m_SubMeshes.emplace_back();
			subMesh.baseVertex = vertexCount;
			subMesh.baseIndex = indexCount;
			subMesh.materialIndex = mesh->mMaterialIndex;
			subMesh.indexCount = mesh->mNumFaces * 3;
			subMesh.meshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += subMesh.indexCount;

			//vertices
			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

				if (mesh->HasTangentsAndBitangents()) {
					vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					vertex.binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}
				if (mesh->HasTextureCoords(0)) {
					vertex.texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}

				vertex.entityID = (int)entityID;

				m_Vertices.push_back(vertex);
			}

			//Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				Face face = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				m_Faces.push_back(face);

			}
		}
		TraverseNodes(scene->mRootNode);


		//Materials
		if (scene->HasMaterials()) {


			//m_Textures.resize(scene->mNumMaterials);
			for (size_t i = 0; i < scene->mNumMaterials; i++) {

				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();

				//auto mi = MakeRef<MaterialInstance>(m_BaseMaterial);

				HZ_CORE_INFO("  Mesh : {0} (Index = {1})", aiMaterialName.data, i);
				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
				HZ_CORE_INFO("    TextureCount = {0}", textureCount);

				aiColor3D aiColor;
				aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

				float shiniess, metalness;
				aiMaterial->Get(AI_MATKEY_SHININESS, shiniess);
				aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness);

				float roughness = 1.0f - glm::sqrt(shiniess / 100.f);
				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				if (hasAlbedoMap) {
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Albedo map path = {0}", texturePath);
					auto texture = Texture2D::Create(texturePath);

					if (texture->IsLoaded()) {
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Albedo);
					}
				}
				else {
					// set material color to aiColor 
				}

				//NormalMaps
				if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS) {
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Normal map path = {0}", texturePath);
					auto texture = Texture2D::Create(texturePath);
					if (texture->IsLoaded()) {
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Normal);
					}
					else {

					}
				}
				else {

				}

				// TODO:Roughness map
				// TODO:Metalness map




				// VertexArray
				m_VertexArray = VertexArray::Create();

				Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
				vertexBuffer->SetLayout(std::vector<BufferElement>{
					BufferElement{ ShaderDataType::Float3, "a_Position" },
						BufferElement{ ShaderDataType::Float3, "a_Normal" },
						BufferElement{ ShaderDataType::Float3, "a_Tangent" },
						BufferElement{ ShaderDataType::Float3, "a_Binormal" },
						BufferElement{ ShaderDataType::Float2, "a_TexCoord" },
						BufferElement{ ShaderDataType::Int, "a_EntityID" }
				}
				);
				m_VertexArray->AddVertexBuffer(vertexBuffer);

				Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Faces.data(), m_Faces.size() * 3);
				m_VertexArray->SetIndexBuffer(indexBuffer);
			}

		}


		m_Type = scene->mAnimations == nullptr ? MeshType::Static : MeshType::Animated;

	}

	Ref<Texture2D> Mesh::GetTexture(TextureType type)
	{
		if (type == TextureType::None) {
			HZ_CORE_WARN("Invalid type of texture");
			return nullptr;
		}

		for (auto texture : m_Textures) {
			if (texture->GetType() == type) {
				return texture;
			}
		}

		HZ_CORE_WARN("Can't find this type of texture. Type : {}", Utils::TextureTypeToString(type));
		return nullptr;
	}

	void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
	{
		glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);

		for (size_t i = 0; i < node->mNumMeshes; i++) {
			uint32_t mesh = node->mMeshes[i];
			auto& subMesh = m_SubMeshes[mesh];
			subMesh.nodeName = node->mName.C_Str();
			subMesh.transform = transform;
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			TraverseNodes(node->mChildren[i], transform, level + 1);
	}

	void Mesh::Submit()
	{
		this->m_VertexArray->Bind();
	}

}