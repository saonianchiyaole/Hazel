#pragma once


namespace Hazel {



	enum class RenderCommandType {


		// Create
		CreateTexture,
		CreateFramebuffer,
		CreateVertexBuffer,
		CreateIndexBuffer,
		CreateUniformBuffer,
		CreateProgram,


		// Update

		UpdateTexture,
		UpdateFramebuffer,
		UpdateVertexBuffer,
		UpdateIndexBuffer,
		UpdateUniformBuffer,
		UpdateProgram,

		// Destroy
		
		DestroyTexture,
		DestroyFramebuffer,
		DestroyVertexBuffer,
		DestroyIndexBuffer,
		DestroyUniformBuffer,
		DestroyProgram,

		Count

	};


	class CommandBuffer
	{

	public:

		void Begin();

		void Submit();

		void End();

		void Execute();

		static Ref<CommandBuffer> Create();

	private:



	};



}