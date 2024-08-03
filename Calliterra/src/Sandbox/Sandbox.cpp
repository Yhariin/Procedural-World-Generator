#include "pch.h"
#include "Sandbox.h"
#include "Sandbox/BasicShapes/Cube.h"
#include "Sandbox/BasicShapes/RadialSphere.h"
#include "Sandbox/BasicShapes/IcoSphere.h"
#include "Sandbox/BasicShapes/Plane.h"
#include "Sandbox/Components/PointLight.h"

Sandbox::Sandbox(float aspectRatio)
	: m_Camera(aspectRatio, 90.f)
{
	Cube::InitBuffers();
		
}

void Sandbox::OnUpdate(float dt)
{
	m_Camera.OnUpdate(dt);
	for (auto& drawable : m_Drawables)
	{
		drawable->SetViewMatrix(m_Camera.GetViewMatrix());
		drawable->SetProjectionMatrix(m_Camera.GetProjectionMatrix());
		drawable->Update(dt);
		drawable->Draw();
	}

}

void Sandbox::OnEvent(Event& e)
{
	m_Camera.OnEvent(e);
}

void Sandbox::LoadSandboxPreset()
{
	CreatePlane();
	for(int i = 0; i < 300; i++)
	{ 
		CreateCube();
	}
	CreatePointLight();
}

void Sandbox::CreateCube()
{
	m_Drawables.emplace_back(std::make_unique<Cube>());
}

void Sandbox::CreateRadialSphere()
{
	m_Drawables.emplace_back( std::make_unique<RadialSphere>() );
}

void Sandbox::CreateIcoSphere()
{
	m_Drawables.emplace_back(std::make_unique<IcoSphere>(4, DX::XMMatrixTranslation(0.f, 20.f, 10.f)));
}

void Sandbox::CreatePlane()
{
	m_Drawables.emplace_back(std::make_unique<Plane>(1));

	m_Drawables.back()->Update(
		DX::XMMatrixTranslation(-0.5f, 0.0f, -0.5f) *
		DX::XMMatrixScaling(100.f, 1.f, 100.f)
	);
}

void Sandbox::CreatePointLight()
{
	m_Drawables.emplace_back(std::make_unique<PointLight>(DX::XMMatrixTranslation(0.f, 20.f, 10.f), DX::XMFLOAT3(1.f, 1.f, 1.f) ));
}
