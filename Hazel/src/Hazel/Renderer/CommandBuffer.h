#pragma once


namespace Hazel {


	class CommandBuffer
	{

	public:

		virtual void Begin() = 0;

		virtual void Submit() = 0;

		virtual void End() = 0;

		static Ref<CommandBuffer> Create();

	private:

	};



}