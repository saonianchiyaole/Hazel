#include "hzpch.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Component.h"

#include <fstream>


namespace YAML {

	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}

	};

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

		HZ_CORE_ASSERT(entity.HasComponent<IDComponent>(), "This Entity doesn't have ID!!!!!!");
		out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetComponent<IDComponent>().ID;

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
			auto& spriteComponent = entity.GetComponent<SpriteComponent>();
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << spriteComponent.color;
			if (spriteComponent.texture)
				out << YAML::Key << "Texture" << YAML::Value << spriteComponent.texture->GetPath();
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CircleRendererComponent>()) {
			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.fade;
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

		if (entity.HasComponent<Rigidbody2DComponent>()) {
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap;
			auto& rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

			out << YAML::Key << "Type" << YAML::Value << (int)rigidbody2D.type;
			out << YAML::Key << "FixedRotation" << YAML::Value << rigidbody2D.fixedRotation;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider2DComponent>()) {
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;
			auto& boxCollider2D = entity.GetComponent<BoxCollider2DComponent>();

			out << YAML::Key << "Offset" << YAML::Value << boxCollider2D.offset;
			out << YAML::Key << "Size" << YAML::Value << boxCollider2D.size;
			out << YAML::Key << "Density" << YAML::Value << boxCollider2D.density;
			out << YAML::Key << "Friction" << YAML::Value << boxCollider2D.friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxCollider2D.restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxCollider2D.restitutionThreshold;
			

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CircleCollider2DComponent>()) {
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap;
			auto& circleCollider2D = entity.GetComponent<CircleCollider2DComponent>();

			out << YAML::Key << "Offset" << YAML::Value << circleCollider2D.offset;
			out << YAML::Key << "Radius" << YAML::Value << circleCollider2D.radius;
			out << YAML::Key << "Density" << YAML::Value << circleCollider2D.density;
			out << YAML::Key << "Friction" << YAML::Value << circleCollider2D.friction;
			out << YAML::Key << "Restitution" << YAML::Value << circleCollider2D.restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << circleCollider2D.restitutionThreshold;


			out << YAML::EndMap;
		}

		if (entity.HasComponent<ScriptComponent>()) {
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;
			auto& script = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ClassName" << YAML::Value << script.className;


			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshComponent>()) {
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;
			auto& mesh = entity.GetComponent<MeshComponent>();

			out << YAML::Key << "FilePath" << YAML::Value << mesh.mesh->GetFilePath();


			out << YAML::EndMap;
		}

		if (entity.HasComponent<MaterialComponent>()) {
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap;
			auto& material = entity.GetComponent<MaterialComponent>();

			out << YAML::Key << "Path" << YAML::Value << material.name;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<LightComponent>()) {
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;
			auto& light = entity.GetComponent<LightComponent>();

			out << YAML::Key << "Color" << YAML::Value << light.color;

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
				
				//Tag
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				
				HZ_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEnttiyWithUUID(name, uuid);

				//Transform
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent) {
					deserializedEntity.GetComponent<TransformComponent>().SetTransform(
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
					auto& sprite = deserializedEntity.AddComponent<SpriteComponent>(
						spriteComponent["Color"].as<glm::vec4>()
					);
					auto texture = spriteComponent["Texture"];
					if (texture)
						sprite.texture = Texture2D::Create(texture.as<std::string>());
				}

				auto circleRenderer = entity["CircleRendererComponent"];
				if (circleRenderer) {
					auto& circle = deserializedEntity.AddComponent<CircleRendererComponent>();
					circle.color = circleRenderer["Color"].as<glm::vec4>();
					circle.thickness = circleRenderer["Thickness"].as<float>();
					circle.fade = circleRenderer["Fade"].as<float>();
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent) {
					auto& rigidbody2D = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rigidbody2D.type = (Rigidbody2DComponent::BodyType)rigidbody2DComponent["Type"].as<int>();
					rigidbody2D.fixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent) {
					auto& boxCollider2D = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					boxCollider2D.offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					boxCollider2D.size = boxCollider2DComponent["Size"].as<glm::vec2>();
					boxCollider2D.density = boxCollider2DComponent["Density"].as<float>();
					boxCollider2D.friction = boxCollider2DComponent["Friction"].as<float>();
					boxCollider2D.restitution = boxCollider2DComponent["Restitution"].as<float>();
					boxCollider2D.restitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent) {
					auto& circleCollider2D = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					circleCollider2D.offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					circleCollider2D.radius = circleCollider2DComponent["Radius"].as<float>();
					circleCollider2D.density = circleCollider2DComponent["Density"].as<float>();
					circleCollider2D.friction = circleCollider2DComponent["Friction"].as<float>();
					circleCollider2D.restitution = circleCollider2DComponent["Restitution"].as<float>();
					circleCollider2D.restitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent) {
					auto& script= deserializedEntity.AddComponent<ScriptComponent>();
					script.className = scriptComponent["ClassName"].as<std::string>();

				}
				//Mesh
				auto meshComponent = entity["MeshComponent"];
				if (meshComponent) {
					auto& mesh = deserializedEntity.AddComponent<MeshComponent>();
					mesh.SetMesh(MakeRef<Mesh>(meshComponent["FilePath"].as<std::string>(), deserializedEntity.GetHandle()));

				}

				//Material
				auto materialComponent = entity["MaterialComponent"];
				if (materialComponent) {
					auto& material = deserializedEntity.AddComponent<MaterialComponent>();
					material.name = materialComponent["Path"].as<std::string>();



					// TODO fix this
					/*if(ShaderLibrary::Exists(material.name))
						material.material = MakeRef<Material>(ShaderLibrary::Get(material.name));
					else {

					}*/

					material.material = MakeRef<Material>(ShaderLibrary::Get(material.name));
				}

				auto lightComponent = entity["LightComponent"];
				if (lightComponent) {
					auto& light = deserializedEntity.AddComponent<LightComponent>();
					light.color = lightComponent["Color"].as<glm::vec3>();
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