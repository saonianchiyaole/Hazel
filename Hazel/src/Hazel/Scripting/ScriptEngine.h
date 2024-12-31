#pragma once


#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/class.h"
#include "mono/metadata/tabledefs.h"

#include "Hazel/Core/Timestep.h"
#include "Hazel/Scene/Entity.h"


namespace Hazel {

	class Scene;


	namespace Utils {
		char* ReadBytes(const std::string& path, uint32_t* outSize);
		void PrintAssemblyTypes(MonoAssembly* assembly);
	}

	class ScriptInstance;
	class ScriptClass;
	class ScriptEngine;


	enum class ScriptFieldType {
		None,
		Float,
		Double,
		Bool,
		Char,
		Short,
		Int,
		Long,
		Byte,
		UShort,
		UInt,
		ULong,
		Vector2,
		Vector3,
		Vector4,
		Entity

	};


	struct ScriptField {
		std::string name;
		ScriptFieldType type;
		MonoClassField* field;
	};

	template<class T>
	struct ScriptFieldInstance {
		T* data;
		~ScriptFieldInstance() {
			delete data; 
		}
	};

	class ScriptInstance {

	public:
		ScriptInstance() = default;
		ScriptInstance(Ref<ScriptClass> scriptClass, uint64_t id);
		~ScriptInstance();
		void InvokeOnCreate();
		void InvokeOnUpdate(Timestep ts);
		void InvokeOnDestroy();

		void Init();

		template<class T>
		T GetFieldValue(std::string name) {
			if (m_ScriptClass->m_Fields.find(name) == m_ScriptClass->m_Fields.end()) {
				HZ_CORE_ASSERT("This field doesn't exist!");
			}

			if (m_FieldInstance.find(name) != m_FieldInstance.end()) {
				T* data = (T*)m_FieldInstance[name];
				mono_field_get_value(m_Instance, m_ScriptClass->m_Fields[name].field, data);
				return *(T*)data;
			}
			else
			{
				T* data = new T;
				mono_field_get_value(m_Instance, m_ScriptClass->m_Fields[name].field, data);
				m_FieldInstance[name] = (T*)data;
				return *(T*)data;
			}
		}

		template<class T>
		void SetFieldVaule(std::string name, T value) {
			if (m_FieldInstance.find(name) != m_FieldInstance.end()) {
				mono_field_set_value(m_Instance, m_ScriptClass->m_Fields[name].field, &value);
			}
			else
				HZ_CORE_ASSERT("This field doesn't exist!");
		}

	private:

		void FreeMemory();

		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance;

		std::unordered_map<std::string, void*> m_FieldInstance;

		friend class ScriptClass;

	};

	class ScriptClass {
	public:

		ScriptClass() = default;
		ScriptClass(const ScriptClass& other) = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		const std::unordered_map<std::string, ScriptField>& GetFields();

		void Initiate();
		MonoObject* CreateInstance();
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params);
	private:

		MonoMethod* m_Constructor;
		MonoMethod* m_OnCreateMethod;
		MonoMethod* m_OnUpdateMethod;
		MonoMethod* m_OnDestroyMethod;


		std::unordered_map<std::string, ScriptField> m_Fields;

		std::string m_Namespace;
		std::string m_Name;

		MonoClass* m_MonoClass = nullptr;
		friend class ScriptInstance;
		friend class ScriptEngine;
	};



	class ScriptEngine {


	public:
		static void Init();
		static void InitMono();
		static void ShutDown();
		static void ShutDownMono();


		//Assembly
		static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath);
		static bool LoadAssembly(const std::string& path);
		static bool LoadAppAssembly(const std::string& path);
		static bool LoadAssemblyClasses(MonoAssembly* assembly);


		//Image
		static MonoImage* GetCoreAssemblyImage();

		//Class
		static MonoClass* CreateMonoClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		static Ref<ScriptClass> CreateScriptClass();
		static Ref<ScriptClass> GetScriptClass(std::string& fullName);
		static MonoObject* InitClass(MonoClass* monoClass);


		//Instance
		static Ref<ScriptInstance> GetScriptInstance(const UUID id);

		//Method
		static MonoMethod* GetMethod(MonoClass* monoClass, const std::string& fullName, int paramCount);


		//Entity
		//static void InitiateEntity(Entity* entity, const std::string& className);
		static void OnCreateEntity(Entity* entity);
		static void OnUpdateEntity(Entity* entity, float ts);
		static void OnDestroyEntity(Entity* entity);
		static bool IsEntityClassExist(std::string& fullName);


		//Scene
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static Scene* GetSceneContext();

	};

}
