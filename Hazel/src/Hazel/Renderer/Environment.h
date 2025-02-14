#pragma once



namespace Hazel {

	class Shader;
	class TextureCube;

	class Environment {
	public:
		static Ref<Environment> Create(std::string filepath);

		Ref<TextureCube> GetEnvironmentMap() { return m_EnvFiltered; }
		Ref<TextureCube> GetIrradianceMap() { return m_IrradianceMap; }

	protected:
		static Ref<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader;

		Ref<TextureCube> m_EnvFiltered;
		Ref<TextureCube> m_IrradianceMap;

	};


}