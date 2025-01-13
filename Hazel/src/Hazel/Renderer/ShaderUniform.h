#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {


	namespace Utils {

		void* AllocateMemoryByShaderDataType(ShaderDataType type, size_t amount = 1);

		template<typename T>
		bool isDataFormatCorrect(ShaderDataType m_Type) {

			switch (m_Type)
			{
			case Hazel::ShaderDataType::None:
				return false;
			case Hazel::ShaderDataType::Sampler2D:

			{
				return std::is_same<T, Hazel::Texture2D*>::value ||
					std::is_same<T, Ref<Texture2D>>::value ||
					std::is_same<T, Hazel::Texture2D>::value;
			}
			case Hazel::ShaderDataType::SamplerCube:
				break;
			default:
				break;
			}

			return sizeof(T) == Utils::ShaderDataTypeSize(m_Type);
		}

	}


	class ShaderUniform {


	protected:

		//void FreeMemory();

		std::string m_Name;
		ShaderDataType m_Type;

	public:

		ShaderUniform();
		ShaderUniform(std::string name, ShaderDataType type);
		virtual ~ShaderUniform();


		/*template<typename T>
		inline T* GetData()
		{
			return (T*)m_Value;
		}

		template<typename T>
		inline void SetData(T data)
		{
			if (Utils::isDataFormatCorrect<T>(m_Type))
				*(T*)m_Value = data;
		}*/

		virtual void Submit(int32_t shaderID, void* data) = 0;


		ShaderDataType GetType();
		std::string GetName();

		static Ref<ShaderUniform> Create(const std::string name, ShaderDataType type);
	};

	class ShaderUniformInstance {

		/*ShaderUniform shaderUniform;
		void* m_Data;*/

	};



}