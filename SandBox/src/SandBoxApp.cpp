#include "Hazel.h"

#include "Hazel/Event/ApplicationEvent.h"

#include "imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Hazel/Renderer/OrthographicCameraController.h"

//#include "Hazel/Core/EntryPoint.h"
#include "Sandbox2D.h"
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.h"

class ExampleLayer : public Hazel::ImGuiLayer {
public:
	ExampleLayer()
		:ImGuiLayer() {


	}

	void OnAttach() override {

	}
	void OnDetach() override {

	}

	void OnUpdate(Hazel::Timestep ts) override {

		//glfw
	}

	virtual void OnImGuiRender() override {

		
	}

	void OnEvent(Hazel::Event& event) override {

		
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& e) {
		//HZ_INFO("{0}", e);
		return false;
	}
private:
};


class Sandbox :public Hazel::Application {
public:
	Sandbox() {
		//PushOverlay(new ExampleLayer());
		PushLayer(new ExampleLayer());
	}

	Sandbox(std::string windowName) : Application(windowName) {

	}

	~Sandbox() {

	}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox("Vulkan Test");
}

int main() {



	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;

}