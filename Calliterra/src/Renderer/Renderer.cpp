#include "pch.h"
#include "Renderer.h"
#include "Platform/DX11/DX11VertexBuffer.h"
#include "Platform/DX11/DX11IndexBuffer.h"
#include "Platform/DX11/DX11Shader.h"
#include "Platform/DX11/DX11Texture.h"
#include "Platform/DX11/DX11Blender.h"

void Renderer::Init(std::shared_ptr<GraphicsContext> graphicsContext)
{
	s_GraphicsContext = graphicsContext;

	s_RendererAPI->Init(s_GraphicsContext);
}

void Renderer::Shutdown()
{
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
	s_GraphicsContext->SetClearColor(r, g, b, a);
}

void Renderer::Clear()
{
	s_GraphicsContext->Clear();
}

void Renderer::Bind(const std::vector<std::shared_ptr<Shader>>& shaderList, 
					const std::shared_ptr<VertexBuffer>& vertexBuffer, 
					const std::shared_ptr<IndexBuffer>& indexBuffer, 
					const std::vector<std::shared_ptr<Texture>>& textureList, 
					const std::vector<std::shared_ptr<ConstantBuffer>>& constantBufferList,
					const std::shared_ptr<Blender>& blender)
{
	for (auto& shader : shaderList)
	{
		shader->Bind();
	}

	if (vertexBuffer != nullptr)
	{
		vertexBuffer->SetLayout();
		vertexBuffer->Bind();
	}

	if (indexBuffer != nullptr)
	{
		indexBuffer->Bind();
		s_IndexCount = indexBuffer->GetCount();
	}

	for (auto& texture : textureList)
	{
		texture->Bind({});
	}

	for (auto& buffer : constantBufferList)
	{
		buffer->Bind({});
	}

	if (blender != nullptr)
	{
		blender->Bind();
	}
}

void Renderer::Draw()
{
	s_RendererAPI->DrawIndexed(s_IndexCount);
}

std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	switch(GetAPI())
	{
	case RendererAPI::None: 
		ASSERT(false, "RendererAPI is set to None!");
		return nullptr;
	case RendererAPI::DX11:
		return std::make_shared<DX11IndexBuffer>(*dynamic_cast<DX11Context*>(s_GraphicsContext.get()), indices);
	}

	LOG_ERROR("Unknown RendererAPI");
	return nullptr;

}

std::shared_ptr<Shader> Renderer::CreateShader(const std::string& filepath, Shader::ShaderType type)
{

	switch(GetAPI())
	{
	case RendererAPI::None: 
		ASSERT(false, "RendererAPI is set to None!");
		return nullptr;
	case RendererAPI::DX11:
		return std::make_shared<DX11Shader>(*dynamic_cast<DX11Context*>(s_GraphicsContext.get()), filepath, type);
	}

	LOG_ERROR("Unknown RendererAPI");
	return nullptr;

}

std::shared_ptr<Texture> Renderer::CreateTexture(const std::string& filepath, uint32_t slot)
{
	switch(GetAPI())
	{
	case RendererAPI::None: 
		ASSERT(false, "RendererAPI is set to None!");
		return nullptr;
	case RendererAPI::DX11:
		return std::make_shared<DX11Texture>(*dynamic_cast<DX11Context*>(s_GraphicsContext.get()), filepath, slot);
	}

	LOG_ERROR("Unknown RendererAPI");
	return nullptr;
}

std::shared_ptr<Blender> Renderer::CreateBlendState(bool enableBlending, Blender::BlendFunc srcBlend, Blender::BlendFunc destBlend, Blender::BlendOp blendOp)
{
	switch(GetAPI())
	{
	case RendererAPI::None: 
		ASSERT(false, "RendererAPI is set to None!");
		return nullptr;
	case RendererAPI::DX11:
		return std::make_shared<DX11Blender>(*dynamic_cast<DX11Context*>(s_GraphicsContext.get()), enableBlending, srcBlend, destBlend, blendOp);
	}

	LOG_ERROR("Unknown RendererAPI");
	return nullptr;
}

RendererResourceLibrary& Renderer::GetResourceLibrary()
{
	return s_ResourceLibrary;
}

