#include "pch.h"
#include "Step.h"
#include "Renderer/Renderer.h"
#include "Renderer/Drawable.h"
#include "Renderer/RenderGraph/Passes/Base/RenderQueuePass.h"

Step::Step(std::string targetPassName)
	: m_TargetPassName(std::move(targetPassName))
{
}

void Step::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_Bindables.push_back(bindable);
}

void Step::AddBindable(std::shared_ptr<IndexBuffer> bindable)
{
	m_Bindables.push_back(bindable);
	m_IndexCount = bindable->GetCount();
}

void Step::AddBindables(const std::vector<std::shared_ptr<Bindable>>& bindables)
{
	for (const auto bindable : bindables)
	{
		m_Bindables.push_back(bindable);
	}
}

void Step::SetIndexCount(uint32_t indexCount)
{
	m_IndexCount = indexCount;
}

void Step::Submit() const
{
	ASSERT(m_TargetPass != nullptr);
	m_TargetPass->Accept(*this);
	//Renderer::GetRenderQueue().Accept(*this, m_TargetPass);
}

void Step::Execute() const
{
	Renderer::Bind(m_Bindables, m_IndexCount);
	Renderer::Draw();
}

void Step::InitializeParentReferences(const Drawable& parent)
{
	for (auto& bindable : m_Bindables)
	{
		bindable->InitializeParentReference(parent);
	}
}

void Step::Link()
{
	ASSERT(m_TargetPass == nullptr);
	m_TargetPass = &Renderer::GetRenderGraph().GetRenderQueue(m_TargetPassName);
}

