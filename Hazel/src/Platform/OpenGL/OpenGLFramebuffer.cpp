#include "hzpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include "glad/glad.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "OpenGLTexture.h"



namespace Hazel {


	namespace Utils {
		
		GLenum GetInternalFormat(TextureFormat format) {
			switch (format)
			{
			case Hazel::TextureFormat::None:
				break;
			case Hazel::TextureFormat::RGBA:
				return GL_RGBA8;
			case Hazel::TextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH24_STENCIL8;
			case Hazel::TextureFormat::R:
				return GL_R32I;
			default:
				break;
			}
		}
		
		GLenum GetFormat(TextureFormat format) {
			switch (format)
			{
			case Hazel::TextureFormat::None:
				break;
			case Hazel::TextureFormat::RGBA:
				return GL_RGBA;
			case Hazel::TextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH_STENCIL;
			case Hazel::TextureFormat::R:
				return GL_RED_INTEGER;
			default:
				break;
			}
		}
		

		GLenum GetAttachmentType(TextureFormat format) {
			switch (format)
			{
			case Hazel::TextureFormat::DEPTH24STENCIL8:
				return GL_DEPTH_STENCIL_ATTACHMENT;
			}
		}

		GLenum GetType(TextureFormat format) {
			switch (format)
			{
			case Hazel::TextureFormat::R:
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




	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec){
		
		for (auto attachment : m_Specification.attachments) {
			if (!Utils::IsDepthFormat(attachment.format)) {
				m_ColorAttachmentFormats.emplace_back(attachment.format);
			}
			else {
				m_DepthAttachmentFormat = attachment.format;
				m_DepthAttachment = Texture2D::PreCreate();
			}
		}

		// Just used for recored the attachemntID;
		m_ColorAttachments.reserve(m_ColorAttachmentFormats.size());
		for (uint32_t i = 0; i < m_ColorAttachmentFormats.size(); i++) {
			m_ColorAttachments.push_back(Texture2D::PreCreate());
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);
	}

	const FramebufferSpecification& OpenGLFramebuffer::GetSpecification()
	{
		return m_Specification;
	}

	void OpenGLFramebuffer::Invalidate() {

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);


		bool isMultisampled = m_Specification.samples > 1 ? true : false;		

		

		// ------------------------------------------------------- Color Attachment ------------------------------------------------------- 
		
		
		if (m_ColorAttachmentFormats.size()) {
			m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
			Utils::CreateTexture(isMultisampled, m_ColorAttachmentIDs.data(), m_ColorAttachments.size());

			std::vector<Ref<OpenGLTexture2D>> openGLTextures = RefVectorStaticCast<OpenGLTexture2D>(m_ColorAttachments);
			for (uint32_t i = 0; i < m_ColorAttachmentFormats.size(); i++) {
				openGLTextures[i]->m_RendererID = m_ColorAttachmentIDs[i];
			}

			for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
				Utils::BindColorTexture(m_Specification.samples, m_Specification.width, m_Specification.height,
									Utils::GetInternalFormat(m_ColorAttachmentFormats[i]), Utils::GetFormat(m_ColorAttachmentFormats[i]), m_ColorAttachmentIDs[i], i);

			}
		}



		// ------------------------------------------------------- Depth Attachment ------------------------------------------------------- 
		
		if (m_DepthAttachmentFormat != TextureFormat::None) {
			
			Utils::CreateTexture(isMultisampled, &m_DepthAttachmentID, 1);
			Utils::BindDepthTexture(m_Specification.samples, m_Specification.width, m_Specification.height,
				Utils::GetInternalFormat(m_DepthAttachmentFormat), Utils::GetAttachmentType(m_DepthAttachmentFormat), m_DepthAttachmentID);

			Ref<OpenGLTexture2D> openGLDepthTexture = std::static_pointer_cast<OpenGLTexture2D>(m_DepthAttachment);
			openGLDepthTexture->m_RendererID = m_DepthAttachmentID;
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
		glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);
		
		m_Specification = spec;
		m_ColorAttachmentIDs.clear();
		m_DepthAttachment = 0;

		RenderCommand::SetViewPort(0, 0, m_Specification.width, m_Specification.height);
		Invalidate();
	}

	void OpenGLFramebuffer::Resize(const glm::vec2 size)
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);
		
		m_ColorAttachmentIDs.clear();
		m_DepthAttachment = 0;

		m_Specification.width = size.x;
		m_Specification.height = size.y;
		RenderCommand::SetViewPort(0, 0, m_Specification.width, m_Specification.height);
		Invalidate();
	}

	void OpenGLFramebuffer::WaitRenderFinished()
	{
		glFinish();
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

		glClearTexImage(m_ColorAttachmentIDs[attachmentIndex], 0, Utils::GetFormat(m_ColorAttachmentFormats[attachmentIndex]), GL_INT, value);
	}

	void OpenGLFramebuffer::ClearAllAttachments()
	{

		for (uint32_t i = 0; i < m_ColorAttachmentIDs.size(); i++) {

			glClearTexImage(m_ColorAttachmentIDs[i], 0,
							Utils::GetFormat(m_ColorAttachmentFormats[i]),
							GL_INT, &m_Specification.attachments[i].clearValue.color);

		}

		if(m_DepthAttachment)
			glClearTexImage(m_DepthAttachmentID, 0,
							Utils::GetFormat(m_Specification.attachments.back().format),
							GL_FLOAT, &m_Specification.attachments.back().clearValue.depthStencil.depth);

	}
	
	const void OpenGLFramebuffer::BindTexture(uint32_t index, uint32_t slot)
	{
		glBindTextureUnit(slot, m_ColorAttachmentIDs[index]);
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