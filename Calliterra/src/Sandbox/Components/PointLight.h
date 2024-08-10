#pragma once
#include "Renderer/Drawable.h"
#include "Sandbox/BasicShapes/IcoSphere.h"
#include "Core/GlobalSettings.h"

class PointLight : public Drawable, public SettingsSubscriber 
{
public:
	PointLight(DX::XMMATRIX transform = DX::XMMatrixIdentity(), DX::XMFLOAT3 color = {-1.f, -1.f, -1.f});
	void Draw() override;

	void OnSettingsUpdate(SettingsType setting) override;
	void Update(float dt) override;

	void InitBuffers();
	void SetViewMatrix(const DX::XMMATRIX& transform) override { m_Sphere.SetViewMatrix(transform); m_ViewMatrix = transform; }
	void SetProjectionMatrix(const DX::XMMATRIX& transform) override { m_Sphere.SetProjectionMatrix(transform); m_ProjectionMatrix = transform; }

private:
	IcoSphere m_Sphere;
	DX::XMFLOAT4 m_Position = { 0.f, 0.f, 0.f, 1.f };
	std::shared_ptr<Shader> m_PixelShader;
	std::shared_ptr<ConstantBuffer> m_PositionConstantBuffer;
};

