#include "hzpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "glad/glad.h"

#include "Hazel/Renderer/RenderCommand.h"

namespace Hazel {


	namespace Utils {
		bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format) {
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				return true;
			default:
				return false;
			}
		}

		GLenum GetInternalFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case Hazel::FramebufferTextureFormat::None:
				break;
			case Hazel::FramebufferTextureFormat::RGBA8:
				return GL_RGBA8;
			case Hazel::FramebufferTextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH24_STENCIL8;
			case Hazel::FramebufferTextureFormat::RED_INTEGER:
				return GL_R32I;
			default:
				break;
			}
		}
		
		GLenum GetFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case Hazel::FramebufferTextureFormat::None:
				break;
			case Hazel::FramebufferTextureFormat::RGBA8:
				return GL_RGBA;
			case Hazel::FramebufferTextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH_STENCIL;
			case Hazel::FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			default:
				break;
			}
		}
		

		GLenum GetAttachmentType(FramebufferTextureFormat format) {
			switch (format)
			{
			case Hazel::FramebufferTextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH_STENCIL_ATTACHMENT;
			}
		}

		GLenum GetType(FramebufferTextureFormat format) {
			switch (format)
			{
			case Hazel::FramebufferTextureFormat::RED_INTEGER:
				return GL_INT;
			}
		}

		GLenum TargetTexture(bool isMultisampled) {
			return isMultisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		void CreateTexture(bool isMultisampled, uint32_t* outID, uint32_t count) {
			glCreateTextures(TargetTexture(isMultisampled), count, outID);
		}

		void BindColorTexture(int samples, int width, int height, GLenum internalFormat, GLenum format, uint32_t ID, size_t index) {

			bool isMultisampled = samples > 1;
			if (isMultisampled) {
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
				glTextureStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else {

				glBindTexture(GL_TEXTURE_2D, ID);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TargetTexture(isMultisampled), ID, 0);
		}
		void BindDepthTexture(int samples, int width, int height, GLenum internalFormat, GLenum attachType, uint32_t ID) {
			bool isMultisampled = samples > 1;
			if (isMultisampled) {
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
				glTextureStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else {

				glBindTexture(GL_TEXTURE_2D, ID);
				//glTextureStorage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, TargetTexture(isMultisampled), ID, 0);
		}
	}




	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		:m_Specification(spec)
	{

		for (auto spec : m_Specification.attachments.attachments) {
			if (!Utils::IsDepthFormat(spec.textureFormat)) {
				m_ColorAttachmentFormats.emplace_back(spec.textureFormat);
			}
			else {
				m_DepthAttachmentFormat = spec.textureFormat;
			}
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	const FramebufferSpecification& OpenGLFramebuffer::GetSpecification()
	{
		return m_Specification;
	}

	void OpenGLFramebuffer::Invalidate() {

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);


		bool isMultisampled = m_Specification.samples > 1 ? true : false;


		if (m_ColorAttachmentFormats.size()) {
			m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
			Utils::CreateTexture(isMultisampled, m_ColorAttachments.data(), m_ColorAttachments.size());
			for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
				Utils::BindColorTexture(m_Specification.samples, m_Specification.width, m_Specification.height,
									Utils::GetInternalFormat(m_ColorAttachmentFormats[i]), Utils::GetFormat(m_ColorAttachmentFormats[i]), m_ColorAttachments[i], i);

			}
		}


		if (m_DepthAttachmentFormat != FramebufferTextureFormat::None) {
			Utils::CreateTexture(isMultisampled, &m_DepthAttachment, 1);
			Utils::BindDepthTexture(m_Specification.samples, m_Specification.width, m_Specification.height,
				Utils::GetInternalFormat(m_DepthAttachmentFormat), Utils::GetAttachmentType(m_DepthAttachmentFormat), m_DepthAttachment);
		}

		if (m_ColorAttachments.size() > 1) {
			HZ_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Framebuffer's size is out of limit");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty()) {
			glDrawBuffer(GL_NONE);
		}


		HZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGLFramebuffer::Resize(const FramebufferSpecification& spec)
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
		m_Specification = spec;
		RenderCommand::SetViewPort(0, 0, m_Specification.width, m_Specification.height);
		Invalidate();
	}
	void OpenGLFramebuffer::Resize(const glm::vec2 size)
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
		
		m_ColorAttachments.clear();
		m_DepthAttachment = 0;

		m_Specification.width = size.x;
		m_Specification.height = size.y;
		RenderCommand::SetViewPort(0, 0, m_Specification.width, m_Specification.height);
		Invalidate();
	}
	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		HZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Index is out of range");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixel;
		glReadPixels(x, y, 1, 1, Utils::GetFormat(m_ColorAttachmentFormats[attachmentIndex]), Utils::GetType(m_ColorAttachmentFormats[attachmentIndex]), &pixel);
		return pixel;
	}
	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, const void* value)
	{
		HZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Index is out of range");

		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::GetFormat(m_ColorAttachmentFormats[attachmentIndex]), GL_INT, value);
	}
	const uint32_t OpenGLFramebuffer::GetColorAttachment(int index)
	{
		return m_ColorAttachments[index];
	}
	const uint32_t OpenGLFramebuffer::GetDpethAttachment()
	{
		return m_DepthAttachment;
	}
	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}
	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}