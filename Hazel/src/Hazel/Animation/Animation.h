#pragma once

#include "Hazel/Renderer/Mesh.h"


struct aiNodeAnim;
struct aiAnimation;
struct aiScene;

namespace Hazel {

	class Mesh;

	class Animation {
	public:
		
		Animation(Ref<Mesh> m_Mesh);
		Animation();

		void OnUpdate(Timestep ts);

		bool IsPlaying() { return m_IsPlaying; }
		void SetPlaying(bool value) { m_IsPlaying = value; }
		
		void Stop() {
			this->m_IsPlaying = false;
		}
		
		void Play() {
			this->m_IsPlaying = true;
		}

		void Play(std::string animationName);
		std::unordered_map<std::string, uint32_t>& GetAnimations() { return m_AnimationNameToIndex; };
		const std::pair<std::string&, uint32_t> GetCurrentAnimation() { return std::pair<std::string&, uint32_t>(m_CurrentAnimationName, m_CurrentAnimationIndex); }

		static Ref<Animation> Create(Ref<Mesh> mesh);
		static Ref<Animation> Create();
	private:

		void ReadMissingBone(const aiAnimation* animation);

		void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
		void BoneTransform(Timestep ts);
		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		uint32_t FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint32_t FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);


		Ref<Mesh> m_Mesh;
		
		bool m_IsPlaying = false;
		float m_WorldTime = 0.0;
		float m_AnimationSpeed = 1.0f;
		float m_AnimationTime = 0.0f;
		const aiScene* m_Scene;
		int m_AnimationAmount = 0;
		uint32_t m_CurrentAnimationIndex = 0;
		std::string m_CurrentAnimationName;
	
		std::unordered_map<std::string, uint32_t> m_AnimationNameToIndex;



		//ToDo  split the animation(s) into animation and animator
		std::string m_Name;

	};

}

