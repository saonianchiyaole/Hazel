#pragma once

#include "Hazel/Scene/Scene.h"

#include  "yaml-cpp/yaml.h"


namespace Hazel {

	class SceneSerializer {
	public:
		SceneSerializer() = default;
		SceneSerializer(Ref<Scene> scene) : m_Scene(scene) {

		}

		bool SerializeEntity(YAML::Emitter& out, Entity& entity);

		bool Serialize(const std::string& filepath);
		bool RuntimeSerialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);
		bool RuntimeDeserialize(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;
	};

}