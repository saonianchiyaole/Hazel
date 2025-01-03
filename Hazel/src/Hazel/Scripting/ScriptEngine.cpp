#include "hzpch.h"

#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Scripting/ScriptGlue.h"

#include <fstream>
#include "Hazel/Core/UUID.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Component.h"
//#define MONO_PATH "G:\Project\visualStudio\Hazel\Editor\mono\lib\mono\4.5"


namespace Hazel {

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "Hazel.Vector2", ScriptFieldType::Vector2 },
		{ "Hazel.Vector3", ScriptFieldType::Vector3 },
		{ "Hazel.Vector4", ScriptFieldType::Vector4 },
		{ "Hazel.Entity", ScriptFieldType::Entity },
	};


	namespace Utils {

		char* ReadBytes(const std::string& path, uint32_t* outSize) {
			std::ifstream stream(path, std::ios::binary | std::ios::ate);
			if (!stream) {
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0) {
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read(buffer, size);
			stream.close();
			*outSize = size;
			return buffer;

		}

		void PrintAssemblyTypes(MonoAssembly* assembly) {
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

			for (int32_t i = 0; i < numTypes; i++) {
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HZ_CORE_TRACE("{}, {}", nameSpace, name);
			}

		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* type) {
			std::string typeName = mono_type_get_name(type);
			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end()) {
				HZ_CORE_ASSERT("Unknown type: {0}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}

		std::string ScriptFieldTypeToString(ScriptFieldType type) {
			switch (type)
			{
			case ScriptFieldType::Float:
				return "System.Single";
			case ScriptFieldType::Double:
				return "System.Double";
			case ScriptFieldType::Bool:
				return "System.Boolean";
			case ScriptFieldType::Char:
				return "System.Char";
			case ScriptFieldType::Short:
				return "System.Int16";
			case ScriptFieldType::Int:
				return "System.Int32";
			case ScriptFieldType::Long:
				return "System.Int64";
			case ScriptFieldType::Byte:
				return "System.Byte";
			case ScriptFieldType::UShort:
				return "System.UInt16";
			case ScriptFieldType::UInt:
				return "System.UInt32";
			case ScriptFieldType::ULong:
				return "System.UInt64";
			case ScriptFieldType::Vector2:
				return "System.Vector2";
			case ScriptFieldType::Vector3:
				return "System.Vector3";
			case ScriptFieldType::Vector4:
				return "System.Vector4";
			case ScriptFieldType::Entity:
				return "System.Entity";

			}
		}
	}



	struct ScriptEngineData {
		MonoDomain* rootDomain = nullptr;
		MonoDomain* appDomain = nullptr;


		MonoAssembly* coreAssembly = nullptr;
		MonoImage* coreAssemblyImage = nullptr;

		MonoAssembly* appAssembly = nullptr;
		MonoImage* appAssemblyImage = nullptr;

		std::filesystem::path coreAssemblyFilepath;
		std::filesystem::path appAssemblyFilepath;

		ScriptClass entityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> entityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> entityInstances;

		bool AssemblyReloadPending = false;

#ifdef HZ_DEBUG
		bool enableDebugging = true;
#else
		bool enableDebugging = false;
#endif
		// Runtime

		Scene* sceneContext = nullptr;

	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();

		ScriptGlue::RegisterFunctions();

		LoadAssemblyClasses(s_Data->coreAssembly);

		ScriptGlue::RegisterComponents();

	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "");

		s_Data->rootDomain = rootDomain;
		s_Data->appDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->appDomain, true);

		LoadAssembly("Resources/Scripts/ScriptCore.dll");


	}

	void ScriptEngine::ShutDown()
	{

	}

	void ScriptEngine::ShutDownMono()
	{
		s_Data->rootDomain = nullptr;
		s_Data->appDomain = nullptr;
		s_Data->appAssembly = nullptr;
		s_Data->coreAssembly = nullptr;
	}


	MonoAssembly* ScriptEngine::LoadCSharpAssembly(const std::string& assemblyPath) {
		uint32_t fileSize = 0;
		char* fileData = Utils::ReadBytes(assemblyPath, &fileSize);

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK) {
			const char* errorMessage = mono_image_strerror(status);
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;
		return assembly;
	}

	bool ScriptEngine::LoadAssembly(const std::string& path)
	{

		// Create an App Domain
		s_Data->appDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->appDomain, true);

		s_Data->coreAssemblyFilepath = path;
		s_Data->coreAssembly = LoadCSharpAssembly(path);
		if (s_Data->coreAssembly == nullptr)
			return false;

		s_Data->coreAssemblyImage = mono_assembly_get_image(s_Data->coreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::string& path) {

		s_Data->appAssemblyFilepath = path;
		s_Data->appAssembly = LoadCSharpAssembly(path);
		if (s_Data->appAssembly == nullptr)
			return false;

		s_Data->appAssemblyImage = mono_assembly_get_image(s_Data->appAssembly);

		return true;
	}

	bool ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->entityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionTable);

		s_Data->entityClass = ScriptClass("Hazel", "Entity", true);

		for (int32_t i = 0; i < numTypes; i++) {
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			std::string fullName;
			if (strlen(nameSpace))
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(s_Data->coreAssemblyImage, nameSpace, name);


			if (monoClass == s_Data->entityClass.m_MonoClass)
				continue;

			bool isSubClassOfEntity = mono_class_is_subclass_of(monoClass, s_Data->entityClass.m_MonoClass, false);
			if (!isSubClassOfEntity)
				continue;

			Ref<ScriptClass> scriptClass = MakeRef<ScriptClass>(nameSpace, name, false);
			s_Data->entityClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);
			HZ_CORE_WARN("{0} has {1} fields:", name, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					HZ_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = { fieldName, fieldType, field };
				}
			}

			HZ_CORE_TRACE("{}, {}", nameSpace, name);
		}
		return true;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->coreAssemblyImage;
	}

	MonoClass* ScriptEngine::CreateMonoClass(const std::string& classNamespace, const std::string& className, bool isCore)
	{
		return mono_class_from_name(isCore ? s_Data->coreAssemblyImage : s_Data->appAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	Ref<ScriptClass> ScriptEngine::CreateScriptClass()
	{
		return nullptr;
	}

	Ref<ScriptClass> ScriptEngine::GetScriptClass(std::string& fullName)
	{
		return s_Data->entityClasses[fullName];
	}

	//Call the C# class constructer
	MonoObject* ScriptEngine::InitClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->appDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;

	}

	Ref<ScriptInstance> ScriptEngine::GetScriptInstance(const UUID id)
	{
		return s_Data->entityInstances[id];
	}

	MonoMethod* ScriptEngine::GetMethod(MonoClass* monoClass, const std::string& fullName, int paramCount)
	{
		return mono_class_get_method_from_name(monoClass, fullName.c_str(), paramCount);
	}

	void ScriptEngine::OnCreateEntity(Entity* entity)
	{
		auto scriptComponnet = entity->GetComponent<ScriptComponent>();
		std::string& className = scriptComponnet.className;
		if (s_Data->entityClasses.find(className) != s_Data->entityClasses.end())
		{
			Ref<ScriptInstance> instance = MakeRef<ScriptInstance>(s_Data->entityClasses[className], entity->GetEntityID());
			UUID entityID = entity->GetEntityID();
			s_Data->entityInstances[entityID] = instance;
			instance->InvokeOnCreate();
		}
	}


	void ScriptEngine::OnUpdateEntity(Entity* entity, float ts)
	{
		auto scriptComponnet = entity->GetComponent<ScriptComponent>();
		std::string& className = scriptComponnet.className;
		if (s_Data->entityInstances.find(entity->GetEntityID()) != s_Data->entityInstances.end())
		{
			auto& instance = s_Data->entityInstances[entity->GetEntityID()];
			instance->InvokeOnUpdate(ts);
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity* entity)
	{
	}

	bool ScriptEngine::IsEntityClassExist(std::string& fullName)
	{
		return s_Data->entityClasses.find(fullName) != s_Data->entityClasses.end();
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{

		s_Data->sceneContext = scene;

		auto view = scene->GetRegistry().view<ScriptComponent>();
		for (auto entityID : view) {
			Entity entity{ entityID, scene };
			OnCreateEntity(&entity);
		}
	}

	void ScriptEngine::OnRuntimeStop()
	{

		s_Data->entityInstances.clear();
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->sceneContext;
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, uint64_t id)
	{
		this->m_ScriptClass = scriptClass;
		m_Instance = m_ScriptClass->CreateInstance();

		void* param = &id;
		m_ScriptClass->InvokeMethod(m_Instance, m_ScriptClass->m_Constructor, &param);
	}

	ScriptInstance::~ScriptInstance()
	{
		FreeMemory();
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
	{
		m_MonoClass = ScriptEngine::CreateMonoClass(classNamespace, className, true);

		if (isCore)
			m_Constructor = mono_class_get_method_from_name(m_MonoClass, ".ctor", 1);
		else
			m_Constructor = mono_class_get_method_from_name(s_Data->entityClass.m_MonoClass, ".ctor", 1);

		m_OnCreateMethod = mono_class_get_method_from_name(m_MonoClass, "OnCreate", 0);
		m_OnUpdateMethod = mono_class_get_method_from_name(m_MonoClass, "OnUpdate", 1);

	}

	const std::unordered_map<std::string, ScriptField>& ScriptClass::GetFields()
	{
		return m_Fields;
	}

	void ScriptClass::Initiate()
	{
	}

	MonoObject* ScriptClass::CreateInstance()
	{
		return mono_object_new(s_Data->appDomain, m_MonoClass);
	}


	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_ScriptClass->m_OnCreateMethod) {
			m_ScriptClass->InvokeMethod(m_Instance, m_ScriptClass->m_OnCreateMethod, nullptr);
		}
	}

	void ScriptInstance::InvokeOnUpdate(Timestep ts)
	{
		if (m_ScriptClass->m_OnUpdateMethod) {
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_ScriptClass->m_OnUpdateMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (m_ScriptClass->m_OnDestroyMethod) {
			m_ScriptClass->InvokeMethod(m_Instance, m_ScriptClass->m_OnDestroyMethod, nullptr);
		}
	}

	void ScriptInstance::Init()
	{
		mono_runtime_object_init(m_Instance);
	}

	void ScriptInstance::FreeMemory()
	{
		for (auto it : m_FieldInstance) {
			switch (m_ScriptClass->GetFields().find(it.first)->second.type) {
			case ScriptFieldType::None:
				continue;
			case ScriptFieldType::Float:
			case ScriptFieldType::Double:
			{
				delete static_cast<float*>(it.second);
				break;
			}
			case ScriptFieldType::Bool:
			{
				bool* ptr = (bool*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Char:
			{
				char* ptr = (char*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Short:
			{
				int16* ptr = (int16*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Int:
			{
				int32_t* ptr = (int32_t*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Long:
			{
				int64_t* ptr = (int64_t*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Byte:
			{
				BYTE* ptr = (BYTE*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::UShort:
			{
				uint16_t* ptr = (uint16_t*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::UInt:
			{
				uint32_t* ptr = (uint32_t*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::ULong:
			{
				uint64_t* ptr = (uint64_t*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Vector2:
			{
				glm::vec2* ptr = (glm::vec2*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Vector3:
			{
				glm::vec3* ptr = (glm::vec3*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Vector4:
			{
				glm::vec4* ptr = (glm::vec4*)it.second;
				delete ptr;
				break;
			}
			case ScriptFieldType::Entity:
				break;
			}
		}
	}




}