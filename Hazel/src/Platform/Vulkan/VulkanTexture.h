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



	class VulkanTexture2D : public Texture2D {

	public:
		VulkanTexture2D();
		VulkanTexture2D(const std::string& filePath);
		VulkanTexture2D(uint32_t width, uint32_t height);
		VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage = TextureUsage::Texture);
		virtual ~VulkanTexture2D();

		inline virtual const uint32_t			GetRendererID() override		{ return 0; }
		inline VkImage							GetRawImage()					{ return m_Image; }
		inline VkImageView						GetImageView()					{ return m_ImageView; }
		inline VkSampler						GetSampler()					{ return m_Sampler; }
		inline VkDescriptorImageInfo			GetDescriptorImageInfo()		{ return m_DescriptorImageInfo; }
		inline VkImageLayout&					GetLayout()  					{ return m_Layout; }
		void									SetLayout(VkImageLayout layout) { m_Layout = layout; }

		virtual void					SetData(const void* data, const uint32_t size) override;
		virtual void					Bind(uint32_t slot = 0) const override {}		

		virtual bool operator ==(Texture& other) const override {
			return m_Image == ((VulkanTexture2D&)other).m_Image;
		}

	private:

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;
		VkDescriptorImageInfo m_DescriptorImageInfo;
		VkImageLayout m_Layout;
		void* m_MappedData;		
	};

	class VulkanTextureCube : public TextureCube {


	};


}
