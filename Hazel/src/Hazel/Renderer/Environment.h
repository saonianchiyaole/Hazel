#pragma once

#include "Hazel/Core/Core.h"


namespace Hazel {

	class Shader;

	class EnvironmentMap {
	public:
		static Ref<EnvironmentMap> Create();
	private:
		static Ref<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader;
	};


}