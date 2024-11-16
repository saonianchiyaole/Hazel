#pragma once
#pragma once

#include "Hazel/Core/Timestep.h"
#include "OrthographicCamera.h"
#include "glad/glad.h"
#include "Shader.h"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {
	struct ParticleProps
	{
		glm::vec2 Position;
		glm::vec2 Velocity, VelocityVariation;
		glm::vec4 ColorBegin, ColorEnd;
		float SizeBegin, SizeEnd, SizeVariation;
		float LifeTime = 1.0f;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem();

		void OnUpdate(Timestep ts);
		void OnRender(OrthographicCamera& camera);

		void Emit(const ParticleProps& particleProps);
	private:
		struct Particle
		{
			glm::vec2 Position;
			glm::vec2 Velocity;
			glm::vec4 ColorBegin, ColorEnd;
			float Rotation = 0.0f;
			float SizeBegin, SizeEnd;

			float LifeTime = 1.0f;
			float LifeRemaining = 0.0f;

			bool Active = false;
		};
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = 999;

		Ref<VertexArray> m_QuadVA;
		Ref<Shader> m_ParticleShader;
		GLint m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;
	};
}