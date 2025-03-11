#include "hzpch.h"
#include "Animation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

namespace Hazel {

	Animation::Animation(Ref<Mesh> mesh)
	{
		if (!mesh->IsAnimated()) {
			m_Mesh = nullptr;
			return;
		}
		m_Mesh = mesh;
		m_Scene = m_Mesh->m_Scene;
		//ReadMissingBone(m_Scene->mAnimations[0]);

		

		for (uint32_t i = 0; i < m_Scene->mNumAnimations; i++) {
			std::string animationName = m_Scene->mAnimations[i]->mName.C_Str();
			animationName = animationName.substr(animationName.find_last_of('|') + 1);
			m_AnimationNameToIndex[animationName] = i;
		}
		m_CurrentAnimationName = m_AnimationNameToIndex.begin()->first;

	}
	Animation::Animation()
	{
		m_Mesh = nullptr;
	}
	
	void Animation::OnUpdate(Timestep ts)
	{
		if (!m_Mesh)
			return;
		m_WorldTime += ts;

		const aiScene* scene = m_Mesh->m_Scene;

		float ticksPerSecond = (float)(scene->mAnimations[m_CurrentAnimationIndex]->mTicksPerSecond != 0 ? scene->mAnimations[m_CurrentAnimationIndex]->mTicksPerSecond : 25.0f) * m_AnimationSpeed;
		m_AnimationTime += ts * ticksPerSecond;
		m_AnimationTime = fmod(m_AnimationTime, (float)scene->mAnimations[m_CurrentAnimationIndex]->mDuration);

		BoneTransform(m_AnimationTime);
	}

	void Animation::Play(std::string animationName)
	{
		if (m_AnimationNameToIndex.find(animationName) != m_AnimationNameToIndex.end()) {
			// error
		}
		if (m_AnimationNameToIndex[animationName] == m_CurrentAnimationIndex) {
			return;
		}
		m_CurrentAnimationIndex = m_AnimationNameToIndex[animationName];
		m_CurrentAnimationName = animationName;
		m_AnimationTime = 0.0f;

	}

	Ref<Animation> Animation::Create(Ref<Mesh> mesh)
	{
		return MakeRef<Animation>(mesh);
	}

	Ref<Animation> Animation::Create()
	{
		return MakeRef<Animation>();
	}

	void Animation::ReadMissingBone(const aiAnimation* animation)
	{
		/*int size = animation->mNumChannels;
		int boneIndex = 0;
		for (size_t i = 0; i < size; i++) {
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (m_Mesh->m_BoneNameToIndex.find(boneName) == m_Mesh->m_BoneNameToIndex.end())
			{
				m_Mesh->m_BoneNameToIndex[boneName] = m_Mesh->m_BoneCount++;
			}
		}*/
	}

	void Animation::ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& parentTransform)
	{
		std::string name(pNode->mName.data);
		const aiAnimation* animation = m_Scene->mAnimations[m_CurrentAnimationIndex];
		glm::mat4 nodeTransform(Utils::Mat4FromAssimpMat4(pNode->mTransformation));
		const aiNodeAnim* nodeAnim = FindNodeAnim(animation, name);

		if (nodeAnim)
		{
			glm::vec3 translation = InterpolateTranslation(AnimationTime, nodeAnim);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

			glm::quat rotation = InterpolateRotation(AnimationTime, nodeAnim);
			glm::mat4 rotationMatrix = glm::toMat4(rotation);

			glm::vec3 scale = InterpolateScale(AnimationTime, nodeAnim);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

			nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
		}

		glm::mat4 transform = parentTransform * nodeTransform;

		if (m_Mesh->m_BoneNameToIndex.find(name) != m_Mesh->m_BoneNameToIndex.end())
		{
			uint32_t BoneIndex = m_Mesh->m_BoneNameToIndex[name];
			m_Mesh->m_BoneInfo[BoneIndex].finalTransformation = m_Mesh->m_InverseTransform * transform * m_Mesh->m_BoneInfo[BoneIndex].boneOffset;
		}

		for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], transform);
	}

	void Animation::BoneTransform(Timestep ts)
	{
		const aiScene* scene = m_Mesh->m_Scene;
		ReadNodeHierarchy(ts, scene->mRootNode, glm::mat4(1.0f));
		for (size_t i = 0; i < m_Mesh->m_BoneCount; i++)
			m_Mesh->m_BoneTransforms[i] = m_Mesh->m_BoneInfo[i].finalTransformation;
	
	}
	const aiNodeAnim* Animation::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
	{
		for (uint32_t i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == nodeName)
				return nodeAnim;
		}
		return nullptr;
	}
	uint32_t Animation::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
				return i;
		}
		return 0;
	}
	uint32_t Animation::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
				return i;
		}
		return 0;
	}
	uint32_t Animation::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
				return i;
		}
		return 0;
	}
	glm::vec3 Animation::InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim)
	{

		if (nodeAnim->mNumPositionKeys == 1) {
			auto v = nodeAnim->mPositionKeys[m_CurrentAnimationIndex].mValue;
			return { v.x, v.y, v.z };
		}

		uint32_t positionIndex = FindPosition(animationTime, nodeAnim);
		uint32_t nextPositonIndex = positionIndex + 1;
		if (nextPositonIndex > nodeAnim->mNumPositionKeys)
			HZ_CORE_WARN("positoin Index uncorrect!");
		float deltaTime = (float)(nodeAnim->mPositionKeys[nextPositonIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
		factor = factor < 0.0f ? 0.0 : factor;
		if (factor > 1.0f)
			HZ_CORE_WARN("factor must be below 1.0f");
		const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
		const aiVector3D& end = nodeAnim->mPositionKeys[nextPositonIndex].mValue;
		aiVector3D delta = end - start;
		auto aiVec = start + factor * delta;
		return { aiVec.x, aiVec.y, aiVec.z };

	}
	glm::quat Animation::InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumRotationKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mRotationKeys[m_CurrentAnimationIndex].mValue;
			return glm::quat(v.w, v.x, v.y, v.z);
		}

		uint32_t rotationIndex = FindRotation(animationTime, nodeAnim);
		uint32_t nextRotationIndex = (rotationIndex + 1);
		//HZ_CORE_ASSERT(NextRotationIndex < nodeAnim->mNumRotationKeys);
		float deltaTime = (float)(nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
		if (factor < 0.0f)
			factor = 0.0f;
		//HZ_CORE_ASSERT(Factor <= 1.0f, "Factor must be below 1.0f");
		const aiQuaternion& startRotationQ = nodeAnim->mRotationKeys[rotationIndex].mValue;
		const aiQuaternion& endRotationQ = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
		auto q = aiQuaternion();
		aiQuaternion::Interpolate(q, startRotationQ, endRotationQ, factor);
		q = q.Normalize();
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::vec3 Animation::InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumScalingKeys == 1) {
			auto v = nodeAnim->mScalingKeys[m_CurrentAnimationIndex].mValue;
			return { v.x, v.y, v.z };
		}

		uint32_t positionIndex = FindScaling(animationTime, nodeAnim);
		uint32_t nextPositonIndex = positionIndex + 1;
		if (nextPositonIndex > nodeAnim->mNumScalingKeys)
			HZ_CORE_WARN("positoin Index uncorrect!");
		float deltaTime = (float)(nodeAnim->mPositionKeys[nextPositonIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mScalingKeys[positionIndex].mTime) / deltaTime;
		factor = factor < 0.0f ? 0.0 : factor;
		if (factor > 1.0f)
			HZ_CORE_WARN("factor must be below 1.0f");
		const aiVector3D& start = nodeAnim->mScalingKeys[positionIndex].mValue;
		const aiVector3D& end = nodeAnim->mScalingKeys[nextPositonIndex].mValue;
		aiVector3D delta = end - start;
		auto aiVec = start + factor * delta;
		return { aiVec.x, aiVec.y, aiVec.z };
	}

}