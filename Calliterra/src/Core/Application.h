#pragma once
#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvents.h"
#include "Events/KeyEvents.h"
#include "DeltaTime.h"
#include "Timer.h"
#include "Sandbox/Sandbox.h"
#include "ImGui/ImGuiManager.h"
#include "Asset/ModelLoader.h"

class Application
{
public:

	Application();
	~Application();

	void OnEvent(Event& e);

	void Run();

	Window& GetWindow() { return *m_Window; }

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnKeyPressed(KeyPressedEvent& e);

	static Application& Get() { return *s_Instance; }
	static DeltaTime GetDeltaTime() { return s_DeltaTime; }

private:
	inline static Application* s_Instance = nullptr;
	std::unique_ptr<Window> m_Window;

	bool m_Running = true;
	bool m_Minimized = false;
	double m_LastFrameTime = 0.0;
	inline static DeltaTime s_DeltaTime = DeltaTime();
	inline static Timer s_ApplicationTimer = Timer();
	std::unique_ptr<Sandbox> m_Sandbox;
	ImGuiManager m_ImGui;
	ModelLoader m_ModelLoader;
};


