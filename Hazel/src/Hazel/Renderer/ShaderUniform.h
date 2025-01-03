#pragma once

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {


	namespace Utils {





		void* AllocateMemoryByShaderDataType(ShaderDataType type, size_t amount = 1);

		template<typename T>
		bool isDataFormatCorrect(ShaderDataType m_Type);
	}


	class ShaderUniform {

	public:

		ShaderUniform();
		ShaderUniform(std::string name, ShaderDataType type);
		~ShaderUniform();


		template<typename T>
		void SetData(T data);

		template<typename T>
		inline T* GetData()
		{
			return (T*)m_Value;
		}

		virtual void Submit(int32_t shaderID) = 0;


		ShaderDataType GetType();
		std::string GetName();

		static Ref<ShaderUniform> Create(const std::string name, ShaderDataType type);

	protected:

		void FreeMemory();

		std::string m_Name;
		ShaderDataType m_Type;
		void* m_Value;

	};



}