#include "hzpch.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"


#include <stb_image.h>
#include <unordered_set>

namespace Hazel {


	namespace Utils {


		VkFormat GetVulkanFormatFromTextureFormat(TextureFormat format) {

			switch (format) {
			case TextureFormat::None:
				HZ_CORE_ASSERT(false, "Invalid texture format!");
			case TextureFormat::R:
				return VK_FORMAT_R8_UNORM;
			case TextureFormat::RG:
				return VK_FORMAT_R8G8_UNORM;
			case TextureFormat::RGB:
				return VK_FORMAT_R8G8B8_UNORM;
			case TextureFormat::RGBA:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case TextureFormat::Float16:
				return VK_FORMAT_R16G16B16_SFLOAT;
			case TextureFormat::DEPTH24STENCIL8:
				return FindSupportDepthFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT,  VK_FORMAT_D32_SFLOAT , VK_FORMAT_D24_UNORM_S8_UINT },
					VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			}

		}
		

		bool IsStencilFormatIncludedByVulkanFormat(VkFormat format) {

			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		bool IsDepthFormatByVulkanFormat(VkFormat format) {

			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || VK_FORMAT_D32_SFLOAT;
		}

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout& oldLayout, VkImageLayout newLayout) {

			if (oldLayout == newLayout) {
				return;
			}

			Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>();
			commandBuffer->Begin();

			VK_IMAGE_ASPECT_DEPTH_BIT;
			VkImageAspectFlags aspectFlag = Utils::IsDepthFormatByVulkanFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			if (Utils::IsStencilFormatIncludedByVulkanFormat(format))
				aspectFlag |= VK_IMAGE_ASPECT_STENCIL_BIT;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;


			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			} 
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {

				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			}
			else {
				HZ_CORE_ASSERT(false, "Unsupport image layout transition!");
			}


			vkCmdPipelineBarrier(commandBuffer->GetRawCommandBuffer(),
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);


			commandBuffer->End();

			commandBuffer->Submit();

			oldLayout = newLayout;

		}

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {

			Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>();
			commandBuffer->Begin();

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(
				commandBuffer->GetRawCommandBuffer(),
				buffer, 
				image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				1, 
				&region);

			commandBuffer->End();

			commandBuffer->Submit();

		}

		void CopyBufferToImage(VkBuffer buffer, Ref<VulkanTexture2D> image) {
			CopyBufferToImage(buffer, image->GetRawImage(), image->GetWidth(), image->GetHeight());
		}
		

		
		VkFormat FindSupportDepthFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
		{

			VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

			for (VkFormat format : candidates) {

				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return format;
				}
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return format;
				}

			}

			HZ_CORE_ASSERT(false, "No supported depth format!");

		}

	}

	VulkanTexture2D::VulkanTexture2D()
	{

	}

	VulkanTexture2D::VulkanTexture2D(const std::string& filePath)
	{
		m_Path = filePath;
		HZ_CORE_INFO("Open Texture filePath {0}", filePath);

		m_Flag = AssetFlag::Loading;

		static const std::unordered_set<std::string> supportedExtensions = { ".jpg", ".jpeg", ".png",
																			".bmp", ".gif", ".hdr", ".tga" };
		std::filesystem::path checkpath = filePath;
		if (supportedExtensions.find(checkpath.extension().string()) == supportedExtensions.end()) {
			HZ_CORE_WARN("This is Not Texture file!");
			return;
		}

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

		stbi_set_flip_vertically_on_load(true); // ¼ÓÔØÍ¼ÏñÊ±·­×ªYÖá


		if (stbi_is_hdr(filePath.c_str())) {
			m_IsHDR = true;
		}


		int width, height, channels;
		stbi_uc* data;

		data = m_IsHDR ? (uint8_t*)stbi_loadf(filePath.c_str(), &width, &height, &channels, 0) : stbi_load(filePath.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;


		if (channels == 4) {
			m_TextureFormat = TextureFormat::RGBA;
		}
		else if (channels == 3 && !m_IsHDR) {
			m_TextureFormat = TextureFormat::RGB;
		}
		else if (channels == 3 && m_IsHDR) {
			m_TextureFormat = TextureFormat::Float16;
		}
		else if (channels == 2) {
			m_TextureFormat = TextureFormat::RG;
		}

		VkDeviceSize imageSize = width * height * channels * (m_IsHDR ? 2 : 1);

		VkDeviceMemory stagingBufferMemory;
		VkBuffer stagingBuffer;
		Utils::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* stagingBufferDataPtr;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &stagingBufferDataPtr);
		memcpy(stagingBufferDataPtr, data, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(data);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_Width);
		imageInfo.extent.height = static_cast<uint32_t>(m_Height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // todo fix this hard code
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		HZ_CORE_ASSERT(vkCreateImage(device, &imageInfo, nullptr, &m_Image) == VK_SUCCESS, "Failed to create vulkan image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_Image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		HZ_CORE_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory) == VK_SUCCESS, "Failed to allocate vulkan Buffer");

		vkBindImageMemory(device, m_Image, m_Memory, 0);


		Utils::TransitionImageLayout(m_Image,
			Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat),
			m_Layout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		Utils::CopyBufferToImage(stagingBuffer, m_Image, width, height);

		Utils::TransitionImageLayout(m_Image,
			Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat),
			m_Layout,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		//clean up
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);


		//image view part

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		HZ_CORE_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView) == VK_SUCCESS,
						"Failed to create image view");


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetProperties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		HZ_CORE_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler) == VK_SUCCESS, "Failed to create sampler");

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		m_Flag = AssetFlag::Valid;

	}


	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{

		m_Width = width;
		m_Height = height;

		m_Flag = AssetFlag::Loading;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

		int channels = 4;

		if (channels == 4) {
			m_TextureFormat = TextureFormat::RGBA;
		}
		else if (channels == 3 && !m_IsHDR) {
			m_TextureFormat = TextureFormat::RGB;
		}
		else if (channels == 3 && m_IsHDR) {
			m_TextureFormat = TextureFormat::Float16;
		}
		else if (channels == 2) {
			m_TextureFormat = TextureFormat::RG;
		}

		VkDeviceSize imageSize = width * height * channels * (m_IsHDR ? 2 : 1);
		

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_Width);
		imageInfo.extent.height = static_cast<uint32_t>(m_Height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // todo fix this hard code
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		HZ_CORE_ASSERT(vkCreateImage(device, &imageInfo, nullptr, &m_Image) == VK_SUCCESS, "Failed to create vulkan image!");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_Image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		HZ_CORE_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory) == VK_SUCCESS, "Failed to allocate vulkan Buffer");

		vkBindImageMemory(device, m_Image, m_Memory, 0);


		Utils::TransitionImageLayout(m_Image,
			Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat),
			m_Layout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//image view part

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		HZ_CORE_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView) == VK_SUCCESS,
			"Failed to create image view");


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetProperties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		HZ_CORE_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler) == VK_SUCCESS, "Failed to create sampler");

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		m_Flag = AssetFlag::Valid;

	}

	VulkanTexture2D::VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureUsage usage) {
		m_Width = width;
		m_Height = height;
		m_IsHDR = false;

		m_Flag = AssetFlag::Loading;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetRawPhysicalDevice();

		m_TextureFormat = format;
		
		int channels; 
		switch (format) {
		case TextureFormat::R:
			channels = 1;
			break;
		case TextureFormat::RG:
			channels = 2;
			break;
		case TextureFormat::RGB:
			channels = 3;
			break;
		case TextureFormat::RGBA:
			channels = 4;
			break;
		case TextureFormat::Float16:
			channels = 4;
			m_IsHDR = true;
			break;
		case TextureFormat::DEPTH24STENCIL8:
			channels = 2;
			break;
		}

		VkDeviceSize imageSize = width * height * channels * (m_IsHDR ? 2 : 1);
		VkFormat imageFormat = Utils::GetVulkanFormatFromTextureFormat(format);


		VkImageUsageFlags vulkanusage;
		if (Utils::IsDepthFormat(format)) {
			vulkanusage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else {
			vulkanusage = usage == TextureUsage::Texture ? VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_Width);
		imageInfo.extent.height = static_cast<uint32_t>(m_Height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = imageFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // todo fix this hard code
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = vulkanusage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		HZ_CORE_ASSERT(vkCreateImage(device, &imageInfo, nullptr, &m_Image) == VK_SUCCESS, "Failed to create vulkan image!");

		// Memory
		{
			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, m_Image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			HZ_CORE_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory) == VK_SUCCESS, "Failed to allocate vulkan Buffer");

			vkBindImageMemory(device, m_Image, m_Memory, 0);
		}
		
	
		//image view part
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = imageFormat;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
	
		//
		if (Utils::IsDepthFormat(format)) {

			Utils::TransitionImageLayout(m_Image,
				imageFormat,
				m_Layout,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			//depth attachment don't need sampler and descriptorImageInfo

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		}
		else {
			
			Utils::TransitionImageLayout(m_Image,
				imageFormat,
				m_Layout,
				usage == TextureUsage::Attachment ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);					
			
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;									
		}
				

		HZ_CORE_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView) == VK_SUCCESS,
			"Failed to create image view");


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = VulkanContext::GetCurrentContext()->GetPhysicalDevice()->GetProperties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		HZ_CORE_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler) == VK_SUCCESS, "Failed to create sampler");

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		m_Flag = AssetFlag::Valid;
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		m_Flag = AssetFlag::Invalid;

		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();
		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_Memory, nullptr);
	}

	void VulkanTexture2D::SetData(const void* data, const uint32_t size)
	{
		int channels;
		switch (m_TextureFormat) {
		case TextureFormat::R:
			channels = 1;
			break;
		case TextureFormat::RG:
			channels = 2;
			break;
		case TextureFormat::RGB:
			channels = 3;
			break;
		case TextureFormat::RGBA:
			channels = 4;
			break;
		case TextureFormat::Float16:
			channels = 4;
			m_IsHDR = true;
			break;
		}

		// todo : temporarily don't consider the situation of set hdr data

		VkDeviceSize imageSize = m_Width * m_Height * channels * (m_IsHDR ? 2 : 1);

		HZ_CORE_ASSERT(size == imageSize, "Input data doesn't match texture's format")		


		VkDevice device = VulkanContext::GetCurrentContext()->GetDevice()->GetRawDevice();

		VkDeviceMemory stagingBufferMemory;
		VkBuffer stagingBuffer;
		Utils::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* stagingBufferDataPtr;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &stagingBufferDataPtr);
		memcpy(stagingBufferDataPtr, data, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		Utils::CopyBufferToImage(stagingBuffer, m_Image, m_Width, m_Height);

		//clean up
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);

		Utils::TransitionImageLayout(m_Image, 
			Utils::GetVulkanFormatFromTextureFormat(m_TextureFormat),
			m_Layout,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}



}