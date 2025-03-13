#include "hzpch.h"

#include "ScriptGlue.h"
#include "mono/metadata/object.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"






namespace Hazel {



#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name) 

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	static void Test() {
		std::cout << "Test : HelloWorld!" << std::endl;
	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "scene pointer is nullptr!");
		Entity entity = scene->GetEntityFormUUID(entityID);
		HZ_CORE_ASSERT(entity, "entity pointer is nullptr!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "");
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}


	static uint64_t Entity_FindEntityByName(MonoString* name) {
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "scene pointer is nullptr!");
		Entity entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetEntityID();
	}

	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* translate) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		*translate = entity.GetComponent<TransformComponent>().translate;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translate) {

		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		entity.GetComponent<TransformComponent>().SetTranslate(*translate);

	}

	static void AnimationComponent_Play(UUID entityID, MonoString* animationName) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		entity.GetComponent<AnimationComponent>().isPlaying = true;
		auto& animation = entity.GetComponent<AnimationComponent>().animation;
		if (animation){
			char* nameCStr = mono_string_to_utf8(animationName);
			animation->Play(nameCStr);
		}
	}
	static void AnimationComponent_Stop(UUID entityID) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		entity.GetComponent<AnimationComponent>().isPlaying = false;
		if (entity.GetComponent<AnimationComponent>().animation) {
			entity.GetComponent<AnimationComponent>().animation->Stop();
		}
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::vec3* rotation) {
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		*rotation = entity.GetComponent<TransformComponent>().rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation) {

		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityFormUUID(entityID);
		entity.GetComponent<TransformComponent>().SetTranslate(*rotation);

	}

	static bool Input_IsKeyDown(KeyCode keycode) {
		return Input::IsKeyPressed(keycode);
	}

	template<typename... Component>
	static void RegisterComponent() {
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Hazel.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					HZ_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);

	}

	template<typename ... Components>
	static void RegisterComponent(ComponentGroup<Components...>) {
		RegisterComponent<Components...>();
	}


	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(Test);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);

		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		HZ_ADD_INTERNAL_CALL(AnimationComponent_Play);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_Stop);
	}

	void ScriptGlue::RegisterComponents()
	{

		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

}