#include "hzpch.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <stb_image.h>
#include <unordered_set>

namespace Hazel {


	namespace Utils {


		VkFormat GetVulkanFormatFromTextureFormat(TextureFormat format) {

			switch (format) {
			case TextureFormat::None:
				HZ_CORE_ASSERT(false, "Invalid texture format!");
			case TextureFormat::R:
				return VK_FORMAT_R8_UINT;
			case TextureFormat::RG:
				return VK_FORMAT_R8G8_UINT;
			case TextureFormat::RGB:
				return VK_FORMAT_R8G8B8_UINT;
			case TextureFormat::RGBA:
				return VK_FORMAT_R8G8B8A8_UINT;
			case TextureFormat::Float16:
				return VK_FORMAT_R16G16B16_SFLOAT;
			}

		}
		

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {

			Ref<VulkanCommandBuffer> commandBuffer = MakeRef<VulkanCommandBuffer>();
			commandBuffer->Begin();

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

	}



	VulkanTexture2D::VulkanTexture2D(const std::string& filePath)
	{
		m_Path = filePath;
		HZ_CORE_INFO("Open Texture filePath {0}", filePath);

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

		VkDeviceSize imageSize = width * height * 4 * (m_IsHDR ? 2 : 1);

		VkDeviceMemory stagingBufferMemory;
		VkBuffer stagingBuffer = Utils::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Utils::CopyBufferToImage(stagingBuffer, m_Image, width, height);


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

		HZ_CORE_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS,
						"Failed to create image view");


	}


	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{

	}



}