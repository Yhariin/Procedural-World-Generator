#include "pch.h"
#include "Sandbox.h"
#include "Sandbox/BasicShapes/Cube.h"
#include "Sandbox/BasicShapes/RadialSphere.h"
#include "Sandbox/BasicShapes/IcoSphere.h"
#include "Sandbox/BasicShapes/Plane.h"
#include "Sandbox/Components/PointLight.h"
#include "Sandbox/Components/Sun.h"
#include "Asset/Model.h"
#include "Asset/ModelLoader.h"

Sandbox::Sandbox(float aspectRatio)
	: m_Camera(aspectRatio, 90.f)
{
		
}

void Sandbox::OnUpdate(float dt)
{
	m_Camera.OnUpdate(dt);
	
	for (auto& drawable : m_Drawables)
	{
		drawable->SetViewMatrix(m_Camera.GetViewMatrix());
		drawable->SetProjectionMatrix(m_Camera.GetProjectionMatrix());
		drawable->Update(dt);
		drawable->Submit();
	}

	Renderer::GetRenderQueue().Execute();
	Renderer::GetRenderQueue().Reset();
}

void Sandbox::OnEvent(Event& e)
{
	m_Camera.OnEvent(e);
}

void Sandbox::LoadSandboxPreset()
{
	//CreatePlane();

	CreatePointLight();
	CreateSun();
	CreateSkyBox();

	//std::shared_ptr<rapidobj::Result> dragonModel = ModelLoader::LoadModelObj("assets/models/dragon.obj");
	//std::shared_ptr<fastgltf::Asset> vaseClayModel = ModelLoader::LoadModelGltf("assets/models/Vase_Clay.gltf");
	//std::shared_ptr<UfbxScene> heartModel = ModelLoader::LoadModelFbx("assets/models/HumanHeart_FBX.fbx");

	//DX::XMMATRIX transform = DX::XMMatrixRotationX(DX::XMConvertToRadians(90)) * DX::XMMatrixTranslation(10.f, 10.f, -10.f);
	DX::XMMATRIX transform2 = DX::XMMatrixIdentity();
	DX::XMMATRIX transform3 = DX::XMMatrixScaling(0.05f, 0.05f, 0.05f);

	//m_Drawables.emplace_back(std::make_unique<Model>(ModelLoader::GetModel("assets/models/nano_textured/nanosuit.obj", transform2, DX::XMFLOAT3(0.2f, 0.4f, 0.9f))));
	m_Drawables.emplace_back(std::make_unique<Model>(ModelLoader::GetModel("assets/models/Sponza/sponza.obj", transform3, DX::XMFLOAT3(0.2f, 0.4f, 0.9f))));
	CreateCube(DX::XMMatrixScaling(5.f, 5.f, 5.f) * DX::XMMatrixTranslation(6.f, 0.f, 0.f));
	CreateCube(DX::XMMatrixScaling(5.f, 5.f, 5.f));

	//m_Drawables.emplace_back(std::make_unique<Model>(ModelLoader::GetModel("assets/models/nanosuit_hierarchical.gltf", transform, DX::XMFLOAT3(0.2f, 0.4f, 0.9f))));

	//m_Drawables.emplace_back(std::make_unique<Model>(dragonModel, DX::XMMatrixTranslation(10.f, 0.f, -10.f), DX::XMFLOAT3(0.8f, 0.3f, 0.8f )));
	//m_Drawables.emplace_back(std::make_unique<Model>(vaseClayModel, DX::XMMatrixTranslation(0.f, 0.f, -10.f), DX::XMFLOAT3(0.2f, 0.3f, 0.78f )));
	//m_Drawables.emplace_back(std::make_unique<Model>(heartModel, DX::XMMatrixTranslation(5.f, 10.f, -10.f), DX::XMFLOAT3(0.2f, 0.3f, 0.78f )));

}

void Sandbox::CreateCube(const DX::XMMATRIX& transform)
{
	m_Drawables.emplace_back(std::make_unique<Cube>(transform));
	dynamic_cast<Cube*>(m_Drawables.back().get())->MakeIndependent();
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

	m_Drawables.back()->SetTransform(
		DX::XMMatrixRotationX(DX::XMConvertToRadians(90)) *
		DX::XMMatrixScaling(10.f, 10.f, 10.f) *
		DX::XMMatrixTranslation(4.f, 15.f, 0.f)

		//DX::XMMatrixTranslation(-0.5f, 0.0f, -0.5f) *
		//DX::XMMatrixScaling(100.f, 1.f, 100.f)
	);
}

void Sandbox::CreatePointLight()
{
	m_Drawables.emplace_back(std::make_unique<PointLight>(DX::XMMatrixTranslation(0.f, 20.f, 10.f), DX::XMFLOAT3(1.f, 1.f, 1.f) ));
}

void Sandbox::CreateSun()
{
	m_Drawables.emplace_back(std::make_unique<Sun>());
}

void Sandbox::CreateSkyBox()
{
	m_Drawables.emplace_back(std::make_unique<Cube>());
	dynamic_cast<Cube*>(m_Drawables.back().get())->MakeSkyBox();
}

const Camera& Sandbox::GetCamera() const
{
	return m_Camera;
}

