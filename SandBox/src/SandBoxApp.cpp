#include <Hazel.h>
//#include <stdio.h>
class Sandbox :public Hazel::Application {
public :
	Sandbox() {

	}
	~Sandbox() {

	}
};

Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}