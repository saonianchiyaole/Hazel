#pragma once



namespace Hazel {



	class Swapchain {

	public:
		static Ref<Swapchain> Create();

		uint32_t GetImageCount()		{ return m_ImageCount; }
		uint32_t GetCurrentFrameIndex()	{ return m_CurrentFrameIndex; }
		uint32_t GetCurrentImageIndex()	{ return m_CurrentImageIndex; }

		virtual uint32_t AcquireNextImage() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;

	protected:

		uint32_t m_ImageCount = 0;
		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;

	};

}


