#include "hzpch.h"

#include "Hazel/Renderer/Mesh.h"
#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/Renderer.h"

#include "Hazel/Asset/AssetManager.h"

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
	}


	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ValidateDataStructure |	// Validation
		aiProcess_LimitBoneWeights; 		// If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1

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


	Mesh::Mesh(const std::string& path, entt::entity entityID)
		:m_FilePath(path), m_BoneCount(0)
	{

		HZ_CORE_INFO("Open Scene filePath {0}", path);
		if (path.empty()) {
			HZ_CORE_WARN("This is Not Scene file!");
			return;
		}

		m_Importer = MakeScope<Assimp::Importer>();
		m_Importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		//m_Importer->SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
		const aiScene* scene = m_Importer->ReadFile(path, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
			HZ_CORE_ERROR("Failed to load mesh fild: {0}", path);

		m_Scene = scene;

		m_IsAnimated = scene->mAnimations != nullptr;
		m_InverseTransform = glm::inverse(Utils::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));





		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		m_SubMeshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; m++) {
			aiMesh* mesh = scene->mMeshes[m];

			Ref<SubMesh> subMesh = m_SubMeshes.emplace_back(MakeRef<SubMesh>());
			subMesh->baseVertex = vertexCount;
			subMesh->baseIndex = indexCount;
			subMesh->materialIndex = mesh->mMaterialIndex;
			subMesh->indexCount = mesh->mNumFaces * 3;
			subMesh->meshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += subMesh->indexCount;
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

				for (unsigned int channel = 0; channel < AI_MAX_NUMBER_OF_COLOR_SETS; channel++) {
					if (!mesh->mColors[channel]) continue;  // pass the invalid channel
					vertex.color = { mesh->mColors[channel][i].r, mesh->mColors[channel][i].g, mesh->mColors[channel][i].b, mesh->mColors[channel][i].a };
				}

				m_Vertices.push_back(vertex);
			}

			//Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				Face face = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				m_Faces.push_back(face);

			}


		}
		// Bones
		if (m_IsAnimated)
		{
			ExtractBones();
		}

		//Initialize all Submeshes
		TraverseNodes(scene->mRootNode); 
		//Materials
		if (scene->HasMaterials()) {

			m_Materials.reserve(scene->mNumMaterials);

			//m_Textures.resize(scene->mNumMaterials);
			for (size_t i = 0; i < scene->mNumMaterials; i++) {


				Ref<Material> material = AssetManager::Create<Material>(Renderer::GetDefaultPBRShader());
			

				auto aiMaterial = scene->mMaterials[i];
				aiString aiMaterialName;
				
				aiMaterial->Get(AI_MATKEY_NAME, aiMaterialName);
				material->SetName(aiMaterialName.C_Str());
				//auto mi = MakeRef<MaterialInstance>(m_BaseMaterial);

				//HZ_CORE_INFO("Mesh : {0} (Index = {1})", aiMaterialName.data, i);
				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
				HZ_CORE_INFO("TextureCount = {0}", textureCount);

				aiColor4D aiColor;
				aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

				material->SetData("u_Albedo", aiColor);

				float shiniess, metalness;
				aiMaterial->Get(AI_MATKEY_SHININESS, shiniess);
				aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness);

				material->SetData("u_Metalness", 0.0);

				float roughness = 1.0f - glm::sqrt(shiniess / 100.f);
				
				material->SetData("u_Roughness", roughness);

				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				if (hasAlbedoMap) {
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Albedo map path = {0}", texturePath);
					auto texture = TextureLibrary::Load(texturePath);
					if (texture->IsLoaded()) {
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Albedo);
					}
					material->SetData("u_AlbedoTex", texture);
					material->SetData("u_UseAlbedoTex", true);
				}
				else {
					// set material color to aiColor 
					material->SetData("u_UseAlbedoTex", false);
				}

				//NormalMaps
				if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS) {
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Normal map path = {0}", texturePath);
					auto texture = TextureLibrary::Load(texturePath);
					if (texture->IsLoaded()) {
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Normal);

						material->SetData("u_NormalTex", texture);
						material->SetData("u_UseNormalTex", true);
					}
					else {
						material->SetData("u_UseNormalTex", false);
					}
				}
				else {

				}

				//Roughness map
				if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Roughness map path = {0}", texturePath);
					auto texture = TextureLibrary::Load(texturePath);
					if (texture->IsLoaded())
					{
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Roughness);

						material->SetData("u_Roughness", texture);
						material->SetData("u_UseRoughness", true);
					}
					else
					{
						material->SetData("u_UseRoughness", false);
						HZ_CORE_ERROR("    Could not load texture: {0}", texturePath);
					}
				}
				else
				{
				}
				// TODO:Metalness map
				/*if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path modelpath = path;
					auto parentPath = modelpath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					HZ_CORE_INFO("Mesh : Roughness map path = {0}", texturePath);
					auto texture = TextureLibrary::Load(texturePath);
					if (texture->IsLoaded())
					{
						m_Textures.push_back(texture);
						texture->SetType(TextureType::Roughness);

						material->SetData("u_Roughness", texture);
						material->SetData("u_UseRoughness", true);
					}
					else
					{
						material->SetData("u_UseRoughness", false);
						HZ_CORE_ERROR("    Could not load texture: {0}", texturePath);
					}
				}
				else
				{
				}*/

				m_Materials.push_back(material);
			}

		}

		
		m_Flag = AssetFlag::Valid;

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
		glm::mat4 transform = parentTransform * Utils::Mat4FromAssimpMat4(node->mTransformation);

		for (size_t i = 0; i < node->mNumMeshes; i++) {
			uint32_t meshIndex = node->mMeshes[i];
			aiMesh* mesh = m_Scene->mMeshes[meshIndex];
			auto& subMesh = m_SubMeshes[meshIndex];
			subMesh->nodeName = node->mName.C_Str();
			subMesh->transform = transform;

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
	
				subMesh->vertices.push_back(vertex);

				for (unsigned int channel = 0; channel < AI_MAX_NUMBER_OF_COLOR_SETS; channel++) {
					if (!mesh->mColors[channel]) continue;  // pass the invalid channel
					vertex.color = { mesh->mColors[channel][i].r, mesh->mColors[channel][i].g, mesh->mColors[channel][i].b, mesh->mColors[channel][i].a };
				}
			}

			//Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				Face face = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				subMesh->faces.push_back(face);

			}

			subMesh->boneInfluences.resize(subMesh->vertices.size());
			int boneIndex = -1;

			if(mesh->mNumBones > 0)
				subMesh->isRigged = true;
			for (size_t i = 0; i < mesh->mNumBones; i++) {

				aiBone* bone = mesh->mBones[i];
				
				std::string boneName = mesh->mBones[i]->mName.C_Str();

				bool allNoneWeight = true;
				for (size_t j = 0; j < bone->mNumWeights; j++) {
					if (bone->mWeights[j].mWeight > 0.0f) {
						allNoneWeight = false;
					}
				}
				if (allNoneWeight)
					continue;

				if (m_BoneNameToIndex.find(boneName) == m_BoneNameToIndex.end())
				{
					// Allocate an index for a new bone
					boneIndex = m_BoneCount;
					m_BoneCount++;
					BoneInfo bi;
					m_BoneInfo.push_back(bi);
					m_BoneInfo[boneIndex].boneOffset = Utils::Mat4FromAssimpMat4(bone->mOffsetMatrix);
					m_BoneInfo[boneIndex].id = boneIndex;

					m_BoneNameToIndex[boneName] = boneIndex;
				}
				else
				{
					boneIndex = m_BoneNameToIndex[boneName];
				}

				for (size_t j = 0; j < bone->mNumWeights; j++)
				{
					int vertexID = bone->mWeights[j].mVertexId;
					float weight = bone->mWeights[j].mWeight;
					subMesh->boneInfluences[vertexID].AddBoneData(boneIndex, weight);
				}
			}


			// VertexArray
			subMesh->vertexArray = VertexArray::Create();

			Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(subMesh->vertices.data(), subMesh->vertices.size() * sizeof(Vertex));
			vertexBuffer->SetLayout(std::vector<BufferElement>{
				BufferElement{ ShaderDataType::Float3, "a_Position" },
					BufferElement{ ShaderDataType::Float3, "a_Normal" },
					BufferElement{ ShaderDataType::Float3, "a_Tangent" },
					BufferElement{ ShaderDataType::Float3, "a_Binormal" },
					BufferElement{ ShaderDataType::Float2, "a_TexCoord" },
					BufferElement{ ShaderDataType::Float4, "a_Color" }
			}
			);
			subMesh->vertexArray->AddVertexBuffer(vertexBuffer);


			if (m_IsAnimated) {
				Ref<VertexBuffer> boneVertexBuffer = VertexBuffer::Create(subMesh->boneInfluences.data(), subMesh->boneInfluences.size() * sizeof(BoneInfluence));
				boneVertexBuffer->SetLayout(std::vector<BufferElement>{
					BufferElement{ ShaderDataType::Int4, "a_BoneIndices" },
						BufferElement{ ShaderDataType::Float4, "a_BoneWeights" },
				}
				);
				subMesh->vertexArray->AddVertexBuffer(boneVertexBuffer);
			}

			Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(subMesh->faces.data(), subMesh->faces.size() * 3);
			subMesh->vertexArray->SetIndexBuffer(indexBuffer);

		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			TraverseNodes(node->mChildren[i], transform, level + 1);
	}

	void Mesh::ExtractBones()
	{

		m_BoneInfluences.resize(m_Vertices.size());
		int boneIndex = -1;
		for (size_t m = 0; m < m_Scene->mNumMeshes; m++)
		{
			aiMesh* mesh = m_Scene->mMeshes[m];

			for (size_t i = 0; i < mesh->mNumBones; i++) {

				aiBone* bone = mesh->mBones[i];
				Ref<SubMesh> submesh = m_SubMeshes[m];

				std::string boneName = mesh->mBones[i]->mName.C_Str();

				bool allNoneWeight = true;
				for (size_t j = 0; j < bone->mNumWeights; j++) {
					if (bone->mWeights[j].mWeight > 0.0f) {
						allNoneWeight = false;
					}
				}
				if (allNoneWeight)
					continue;

				if (m_BoneNameToIndex.find(boneName) == m_BoneNameToIndex.end())
				{
					// Allocate an index for a new bone
					boneIndex = m_BoneCount;
					m_BoneCount++;
					BoneInfo bi;
					m_BoneInfo.push_back(bi);
					m_BoneInfo[boneIndex].boneOffset = Utils::Mat4FromAssimpMat4(bone->mOffsetMatrix);


					bi.id = boneIndex;
					m_BoneNameToIndex[boneName] = boneIndex;
				}
				else
				{
					boneIndex = m_BoneNameToIndex[boneName];
				}

				for (size_t j = 0; j < bone->mNumWeights; j++)
				{
					int VertexID = submesh->baseVertex + bone->mWeights[j].mVertexId;
					float Weight = bone->mWeights[j].mWeight;
					m_BoneInfluences[VertexID].AddBoneData(boneIndex, Weight);
				}
			}
		}

		m_BoneTransforms.resize(m_BoneCount, glm::mat4(1.0f));
	}

}