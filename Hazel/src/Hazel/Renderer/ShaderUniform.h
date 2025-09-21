#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {


	namespace Utils {

		void* AllocateMemoryByShaderDataType(ShaderDataType type, size_t amount = 1);
		void FreeMemoryByShaderDataType(ShaderDataType type, void* data);
		uint64_t GetAllocatedMemoryByShaderDataType(ShaderDataType type);

		

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