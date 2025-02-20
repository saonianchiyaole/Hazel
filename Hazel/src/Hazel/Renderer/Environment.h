#pragma once



namespace Hazel {

	class Shader;
	class TextureCube;
	class Texture2D;

	class Environment {
	public:
		static Ref<Environment> Create(std::string filepath);

		Ref<TextureCube> GetEnvironmentMap() { return m_EnvFiltered; }
		Ref<TextureCube> GetIrradianceMap() { return m_IrradianceMap; }
		static Ref<Texture2D> GetBRDFLUT() { return s_BRDFLUT; }

		bool IsLoaded() { return m_IsLoaded; }
	protected:
		static Ref<Shader> s_EquirectangularConversionShader, s_EnvFilteringShader, s_EnvIrradianceShader, s_GenerateBRDFLUTShader;

		static Ref<Texture2D> s_BRDFLUT;

		Ref<TextureCube> m_EnvFiltered;
		Ref<TextureCube> m_IrradianceMap;

		bool m_IsLoaded = false;

	};


}