#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Resource/Resource.h"
#include "Hazel/Resource/ResourceManager.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/CommandBuffer.h"
#include "Hazel/Renderer/Environment.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Material.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/RenderPass.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Mesh.h"
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Hazel {


	struct UniformCache {

		std::unordered_map<std::string, Buffer> uniforms;
		std::unordered_map<std::string, std::vector<Handle<Texture2D>>> textures;

	};


	struct RenderDeviceInfo {

		enum class API {
			None = 0,
			OpenGL = 1,
			Vulkan = 2
		};

		API api = API::Vulkan;
		uint32_t frameInFlight = 3;

	};

	class RenderDevice {

	public:
		virtual ~RenderDevice() = default;

		static void SetCurrent(RenderDevice* device);
		static RenderDevice& Get();

		static RenderDevice* Create();

		template<typename T>
		Ref<T> GetResource(const Handle<T>& handle) const {
			return ResourceManager<T>::Get(handle);
		}

		template<typename T>
		bool HasResource(const Handle<T>& handle) const {
			return ResourceManager<T>::Has(handle);
		}
		
		template<typename T>
		void CommitHandle(const Handle<T>& handle, const Ref<T>& resource) const {
			ResourceManager<T>::SetResource(handle, resource);
		}

		virtual bool Init() = 0;
		virtual void CreateVertexBuffer(Handle<VertexBuffer> handle, size_t size, std::vector<uint8_t> data = {}) = 0;				
		virtual void CreateIndexBuffer(Handle<IndexBuffer> handle, size_t size, std::vector<uint8_t> data = {}) = 0;
		virtual void CreateUniformBuffer(Handle<UniformBuffer> handle, const std::string name) = 0;
		virtual void CreateUniformBufferSet(Handle<UniformBufferSet> handle, uint32_t amount, size_t size) = 0;
		virtual void CreateUniformBufferSet(Handle<UniformBufferSet> handle, size_t size) = 0;
		virtual void CreateVertexArray(Handle<VertexArray> handle) = 0;
		virtual void CreateCommandBuffer(Handle<CommandBuffer> handle) = 0;
		virtual void CreateTexture2D(Handle<Texture2D> handle, TextureInfo textureInfo, std::vector<uint8_t> data = {}) = 0;
		/*virtual void CreateTextureCube(Handle<TextureCube> handle, std::vector<Ref<Texture2D>> textures) = 0;
		virtual void CreateTextureCube(Handle<TextureCube> handle, const std::string& path) = 0;
		virtual void CreateTextureCube(Handle<TextureCube> handle, PixelFormat format, uint32_t width, uint32_t height) = 0;
		virtual void CreateTextureCube(Handle<TextureCube> handle) = 0;*/
		virtual void CreateShader(Handle<Shader> handle, const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
		virtual void CreateShader(Handle<Shader> handle, const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
		virtual void CreateShader(Handle<Shader> handle, const std::string& filepath) = 0;
		virtual void CreateFramebuffer(Handle<Framebuffer> handle, const FramebufferInfo info, std::vector<Handle<Texture2D>> attachments = {}) = 0;
		virtual void CreateEnvironment(Handle<Environment> handle, const std::string& filepath) = 0;


		// ------------------------------------------------------------------------------------- Draw -------------------------------------------------------------------------------------------

		virtual void SetViewport(const glm::vec4 viewport) = 0;

		virtual void BeginFrame() = 0;

		virtual void EndFrame() = 0;

		virtual void BindPipeline(Handle<Pipeline> pipeline) = 0;

		virtual void BeginRenderPass(Handle<RenderPass> renderPass) = 0;

		virtual void EndRenderPass() = 0 ;

		virtual void Draw(Handle<Mesh> mesh, Handle<Material> material) = 0;

		virtual void DrawIndexed(const Handle<VertexArray>& vertexArray, uint32_t count) = 0;



		// ------------------------------------------------------------------------------------ Uniform ----------------------------------------------------------

		template<typename T>
		bool SetData(const std::string& name, const T value, uint32_t index = 0) {
			if constexpr (std::is_same_v<std::remove_cv_t<T>, Handle<Texture2D>>) {
				return SetTexture(name, value, index);
			}
			else {
				size_t size = sizeof(T);
				auto [uniformIt, inserted] = m_UniformCache.uniforms.try_emplace(name);
				Buffer& buffer = uniformIt->second;
				if (buffer.GetSize() != size) {
					buffer.Allocate(size);
				}

				buffer.Write(data, size);
				return true;
			}
		}

		bool SetData(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0) {
			return SetTexture(name, texture, index);
		}

		template<typename T>
		bool SetData(const std::string& name, T* value, uint32_t index = 0) {
			if (!value) {
				HZ_CORE_ERROR("Uniform data pointer with name {0} is null!", name);
				return false;
			}

			if constexpr (std::is_same_v<std::remove_cv_t<T>, Texture2D>) {
				HZ_CORE_ERROR("Texture uniform {0} cannot be set from Texture2D*. Use Handle<Texture2D> or Ref<Texture2D> instead.", name);
				return false;
			}
			else {

				size_t size = sizeof(T);
				auto [uniformIt, inserted] = m_UniformCache.uniforms.try_emplace(name);
				Buffer& buffer = uniformIt->second;
				if (buffer.GetSize() != size) {
					buffer.Allocate(size);
				}

				buffer.Write(value, size);
				return true;
			}
		}

		template<typename T>
		bool SetData(const std::string& name, const Ref<T>& value, uint32_t index = 0) {
			if (!value) {
				HZ_CORE_ERROR("Uniform ref with name {0} is null!", name);
				return false;
			}

			if constexpr (std::is_same_v<T, Texture2D>) {
				return SetTexture(name, ResourceManager<Texture2D>::Add(value), index);
			}
			else {

				size_t size = sizeof(T);

				auto [uniformIt, inserted] = m_UniformCache.uniforms.try_emplace(name);
				Buffer& buffer = uniformIt->second;
				if (buffer.GetSize() != size) {
					buffer.Allocate(size);
				}

				buffer.Write(value.get(), size);
				return true;
			}
		}

		bool SetTexture(const std::string& name, const Handle<Texture2D>& texture, uint32_t index = 0) {
			
			if(m_UniformCache->textures.find(name) == m_UniformCache->textures.end()) {
				m_UniformCache->textures[name] = {};
				return true;
			}
			
			if (m_UniformCache->textures[name].size() <= index) {
				m_UniformCache->textures[name].resize(index);
			}

			m_UniformCache->textures[name][index] = texture;
			return true;
		}

		const Ref<UniformCache>& GetUniformCache() const { return m_UniformCache; }
		Ref<UniformCache>& GetUniformCache() { return m_UniformCache; }




	protected:
		template<typename T>
		Handle<T> RegisterResource(const Ref<T>& resource) const {
			HZ_CORE_ASSERT(resource, "RenderDevice failed to create resource");
			return ResourceManager<T>::Add(resource);
		}

		template<typename T>
		void RegisterResource(const Handle<T>& handle, const Ref<T>& resource) const {
			HZ_CORE_ASSERT(resource, "RenderDevice failed to create resource");
			ResourceManager<T>::SetResource(handle, resource);
		}
		

	protected:
		inline static RenderDevice* s_Current = nullptr;
		
		Ref<UniformCache> m_UniformCache = nullptr;
	

		uint32_t m_FrameIndex = 0;
		uint32_t m_FrameInFlight = 3;

	};

}
