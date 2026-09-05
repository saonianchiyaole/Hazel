#pragma once


#include "Hazel/Renderer/Texture.h"
#include "vulkan/vulkan.h"


namespace Hazel {


	class VulkanTexture2D;


	namespace Utils {

		VkFormat		GetVulkanFormatFromTextureFormat		(TextureFormat format);		


		void			TransitionImageLayout					(VkImage image, VkFormat format,
																VkImageLayout& oldLayout, VkImageLayout newLayout);

		void			CopyBufferToImage						(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void			CopyBufferToImage						(VkBuffer buffer, Ref<VulkanTexture2D> image);
		
		VkFormat		FindSupportDepthFormat					(const std::vector<VkFormat>& candidates,
																VkImageTiling tiling, 
																VkFormatFeatureFlags features);

		bool			IsStencilFormatIncludedByVulkanFormat	(VkFormat format);		
	}



	class VulkanTexture2D {

	public:
				

		VulkanTexture2D(const TextureInfo& info, std::vector<uint8_t> data = {});
		~VulkanTexture2D();
		

		bool Init(const TextureInfo& info);

		const TextureInfo& GetInfo();
		
		inline VkImage					GetRawImage()					{ return m_Image; }
		inline VkImageView				GetImageView()					{ return m_ImageView; }
		inline VkSampler				GetSampler()					{ return m_Sampler; }
		inline VkDescriptorImageInfo	GetDescriptorImageInfo()		{ return m_DescriptorImageInfo; }
		inline VkImageLayout&			GetLayout()  					{ return m_Layout; }
		void							SetLayout(VkImageLayout layout) { m_Layout = layout; }

		void							SetData(const void* data, const uint32_t size);	
		void							Bind(uint32_t slot = 0) const {}

		bool operator ==(const VulkanTexture2D& other) const {
			return m_Image == other.m_Image;
		}

	private:

		TextureInfo m_Info;

		VkImage					m_Image;
		VkImageView				m_ImageView;
		VkDeviceMemory			m_Memory;
		VkSampler				m_Sampler;
		VkDescriptorImageInfo	m_DescriptorImageInfo;
		VkImageLayout			m_Layout;
		void*					m_MappedData;


		
	};

	class VulkanTextureCube {


	};


}
