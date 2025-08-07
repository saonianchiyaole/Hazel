#pragma once


#include "Hazel/Renderer/Texture.h"
#include "vulkan/vulkan.h"


namespace Hazel {


	class VulkanTexture2D;


	namespace Utils {

		VkFormat		GetVulkanFormatFromTextureFormat		(TextureFormat format);		


		void			TransitionImageLayout					(VkImage image, VkFormat format,
																VkImageLayout oldLayout, VkImageLayout newLayout);

		void			CopyBufferToImage						(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void			CopyBufferToImage						(VkBuffer buffer, Ref<VulkanTexture2D> image);
				

	}



	class VulkanTexture2D : public Texture2D {

	public:

		VulkanTexture2D(const std::string& filePath);
		VulkanTexture2D(uint32_t width, uint32_t height);
		virtual const uint32_t GetRendererID() override;
		VkImage  GetRawImage() { return m_Image; }

	private:

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		void* m_MappedData;		
	};



}
