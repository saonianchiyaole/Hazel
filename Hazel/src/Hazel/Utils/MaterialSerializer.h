
#include "Hazel/Renderer/Material.h"

#include  "yaml-cpp/yaml.h"


namespace Hazel {


	class ShaderUniform;

	namespace Utils {

		class MaterialSerializer {

		public:
			MaterialSerializer() = default;
			MaterialSerializer(Ref<Material> material) : m_Material(material) {

			}

			bool Serialize(const std::string& filepath);
			bool Deserialize(const std::string& filepath);
			bool SerializeUniform(YAML::Emitter& out, Ref<ShaderUniform> shaderUniform, void * data);
			void DeserializeUniform(Ref<ShaderUniform> shaderUniform, void* data);

		private:
			Ref<Material> m_Material;
			
		};

	}

}