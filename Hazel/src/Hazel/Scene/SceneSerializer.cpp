#include "hzpch.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Component.h"

#include <fstream>


namespace YAML {

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();

			return true;
		}

	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}

	};

	inline YAML::Emitter& operator << (YAML::Emitter& out, glm::vec2 value) {
		out << YAML::Flow;
		out << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator << (YAML::Emitter& out, glm::vec3 value) {
		out << YAML::Flow;
		out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator << (YAML::Emitter& out, glm::vec4 value) {
		out << YAML::Flow;
		out << YAML::BeginSeq << value.x << value.y << value.z << value.w << YAML::EndSeq;
		return out;
	}

}





namespace Hazel {


	bool SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity& entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << "463782467832";

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().tag;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<TransformComponent>()) {

			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& transformComponent = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translate" << YAML::Value << transformComponent.translate;
			out << YAML::Key << "Rotation" << YAML::Value << transformComponent.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transformComponent.scale;

			out << YAML::EndMap;
		}
		if (entity.HasComponent<SpriteComponent>()) {
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << entity.GetComponent<SpriteComponent>().color;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<CameraComponent>()) {
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& camera = entity.GetComponent<CameraComponent>().camera;

			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetProjectionType();
			out << YAML::Key << "Primary" << YAML::Value << entity.GetComponent<CameraComponent>().primary;
			out << YAML::Key << "Fovy" << YAML::Value << camera->GetFovy();
			out << YAML::Key << "ZoomLevel" << YAML::Value << camera->GetZoomLevel();
			out << YAML::Key << "AspectRatio" << YAML::Value << camera->GetAspectRatio();
			out << YAML::Key << "NearClip" << YAML::Value << camera->GetNearClip();
			out << YAML::Key << "FarClip" << YAML::Value << camera->GetFarClip();

			out << YAML::EndMap;
		}


		out << YAML::EndMap;

		return true;
	}

	bool SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::BeginSeq;
		auto view = m_Scene->GetRegistry().view<entt::entity>();
		for (auto& entityID : view) {
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return false;
			SerializeEntity(out, entity);

		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;

	}

	bool SceneSerializer::RuntimeSerialize(const std::string& filepath)
	{
		return false;
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream fin(filepath);
		std::stringstream strStream;
		strStream << fin.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		HZ_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities) {
			for (auto entity : entities) {
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				//Tag
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				HZ_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntity(name);
				//Transform
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent) {
					deserializedEntity.AddComponent<TransformComponent>(
						transformComponent["Translate"].as<glm::vec3>(),
						transformComponent["Rotation"].as<glm::vec3>(),
						transformComponent["Scale"].as<glm::vec3>()
					);
				}
				//Camera
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent) {
					auto& camera = deserializedEntity.AddComponent<CameraComponent>().camera;
					deserializedEntity.GetComponent<CameraComponent>().primary = cameraComponent["Primary"].as<bool>();
					//camera->SetProjectionType(Camera::ProjectionType(cameraComponent["ProjectionType"].as<int>()));
					//camera->SetFovy(cameraComponent["Fovy"].as<float>());
					//camera->SetAspectRatio(cameraComponent["AspectRatio"].as<float>());
					//camera->SetZoomLevel(cameraComponent["ZoomLevel"].as<float>());
					//camera->SetNearClip(cameraComponent["NearClip"].as<float>());
					//camera->SetFarClip(cameraComponent["FarClip"].as<float>());
					camera->m_Type = Camera::ProjectionType(cameraComponent["ProjectionType"].as<int>());
					camera->m_Fovy = cameraComponent["Fovy"].as<float>();
					camera->m_ZoomLevel = cameraComponent["ZoomLevel"].as<float>();
					camera->m_AspectRatio = cameraComponent["AspectRatio"].as<float>();
					camera->m_NearClip = cameraComponent["NearClip"].as<float>();
					camera->m_FarClip = cameraComponent["FarClip"].as<float>();

					camera->RecalculateProjectionMatrix();


				}
				//Sprite
				auto spriteComponent = entity["SpriteComponent"];
				if (spriteComponent) {
					deserializedEntity.AddComponent<SpriteComponent>(
						spriteComponent["Color"].as<glm::vec4>()
					);
				}

			}
		}

		return true;
	}

	bool SceneSerializer::RuntimeDeserialize(const std::string& filepath)
	{
		return false;
	}

}