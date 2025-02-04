#pragma once
#include "DX11Context.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/BufferLayout.h"
#include "Platform/DX11/DX11Shader.h"

static DXGI_FORMAT ShaderDataTypeToD3D(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
	case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
	case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
	case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case ShaderDataType::Int:		return DXGI_FORMAT_R32_SINT;
	case ShaderDataType::Int2:		return DXGI_FORMAT_R32G32_SINT;
	case ShaderDataType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
	case ShaderDataType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
	}

	ASSERT(false, "ShaderType Conversion not supported yet.");
	return DXGI_FORMAT_UNKNOWN;
}

template<typename Type>
class DX11VertexBuffer : public VertexBuffer
{
public:
	DX11VertexBuffer(const DX11Context& context, const std::vector<Type>& vertices)
		: m_DX11Context(context), m_VertexLists(std::vector<std::vector<Type>>{vertices}), m_BufferCount(1)
	{
		InitSingleVertexBuffer();
	}

	DX11VertexBuffer(const DX11Context& context, const std::vector<std::vector<Type>>& listOfVertexArrays)
		: m_DX11Context(context), m_VertexLists(listOfVertexArrays), m_BufferCount(listOfVertexArrays.size())
	{
		InitMultiVertexBuffers();
	}

	void DX11VertexBuffer<Type>::Bind() const override
	{
		ASSERT(m_HasLayout, "Attempting to bind vertex buffer before a layout has been created.");
		m_DX11Context.GetDeviceContext().IASetInputLayout(m_D3DBufferLayout.Get());

		std::vector<UINT> strideList;
		strideList.reserve(m_BufferCount);
		std::vector<UINT> offsets;
		for (uint32_t i = 0; i < m_BufferCount; i++)
		{
			size_t strd = m_BufferLayoutArray[i].GetStride();
			strideList.push_back(static_cast<UINT>(m_BufferLayoutArray[i].GetStride()));
			offsets.push_back(0);
		}

		m_DX11Context.GetDeviceContext().IASetVertexBuffers(0, static_cast<UINT>(m_BufferCount), m_D3DVertexBufferArray[0].GetAddressOf(), &strideList[0], offsets.data());
			
	}

	void DX11VertexBuffer<Type>::CreateLayout(const VertexBufferLayout& layout, Shader* shader) override
	{
		m_BufferLayoutArray[0] = layout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		std::vector<BufferElement> layoutElements = layout.GetElements();

		ComPtr<ID3DBlob> shaderByteCode = dynamic_cast<DX11Shader*>(shader)->GetCompiledShaderByteCode();

		for (int i = 0; i < layoutElements.size(); i++)
		{
			D3D11_INPUT_ELEMENT_DESC tmp = {};
			tmp.SemanticName = layoutElements[i].Name.c_str();
			tmp.SemanticIndex = layoutElements[i].NameIndex;
			tmp.Format = ShaderDataTypeToD3D(layoutElements[i].Type);
			tmp.InputSlot = 0;
			tmp.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			tmp.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			tmp.InstanceDataStepRate = 0;

			desc.push_back(tmp);
		}

	ASSERT_HR(
			m_DX11Context.GetDevice().CreateInputLayout(
				&desc[0],
				static_cast<UINT>(desc.size()),
				shaderByteCode->GetBufferPointer(),
				shaderByteCode->GetBufferSize(),
				&m_D3DBufferLayout
			)
		);

		m_HasLayout = true;

	}

	void DX11VertexBuffer<Type>::CreateLayoutList(const std::vector<VertexBufferLayout>& layoutList, Shader* shader) override
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		ComPtr<ID3DBlob> shaderByteCode = dynamic_cast<DX11Shader*>(shader)->GetCompiledShaderByteCode();

		for (int i = 0; i < layoutList.size(); i++)
		{
			m_BufferLayoutArray[i] = layoutList[i];
			std::vector<BufferElement> layoutElements = layoutList[i].GetElements();

			for (int j = 0; j < layoutElements.size(); j++)
			{
				D3D11_INPUT_ELEMENT_DESC tmp = {};
				tmp.SemanticName = layoutList[i].GetElements()[j].Name.c_str();
				tmp.SemanticIndex = layoutElements[j].NameIndex;
				tmp.Format = ShaderDataTypeToD3D(layoutElements[j].Type);
				tmp.InputSlot = i;
				tmp.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				tmp.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				tmp.InstanceDataStepRate = 0;

				desc.emplace_back(tmp);
			}
		}

		ASSERT_HR(
			m_DX11Context.GetDevice().CreateInputLayout(
				&desc[0],
				static_cast<UINT>(desc.size()),
				shaderByteCode->GetBufferPointer(),
				shaderByteCode->GetBufferSize(),
				&m_D3DBufferLayout
			)
		);


		m_HasLayout = true;

	}

private:
	void InitSingleVertexBuffer()
	{
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.ByteWidth = static_cast<UINT>(sizeof(Type) * m_VertexLists[0].size());
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.CPUAccessFlags = 0;
		vbDesc.MiscFlags = 0;
		vbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexInitData = {};
		vertexInitData.pSysMem = m_VertexLists[0].data();

		ASSERT_HR(
			m_DX11Context.GetDevice().CreateBuffer(
				&vbDesc,
				&vertexInitData,
				&m_D3DVertexBufferArray[0]
			)
		);
	}

	void InitMultiVertexBuffers()
	{
		ASSERT(m_BufferCount < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, "Too many buffers are being bound!");

		for (uint32_t i = 0; i < m_BufferCount; i++)
		{
			D3D11_BUFFER_DESC vbDesc = {};
			vbDesc.Usage = D3D11_USAGE_DEFAULT;
			vbDesc.ByteWidth = static_cast<UINT>(sizeof(Type) * m_VertexLists[i].size());
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbDesc.CPUAccessFlags = 0;
			vbDesc.MiscFlags = 0;
			vbDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA vertexInitData = {};
			vertexInitData.pSysMem = m_VertexLists[i].data(); 

			ASSERT_HR(
				m_DX11Context.GetDevice().CreateBuffer(
					&vbDesc,
					&vertexInitData,
					&m_D3DVertexBufferArray[i]
				)
			);
		}

	}

private:
	const DX11Context& m_DX11Context;

	const std::vector<std::vector<Type>> m_VertexLists;
	const uint32_t m_BufferCount;

	ComPtr<ID3D11Buffer> m_D3DVertexBufferArray[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	ComPtr<ID3D11InputLayout> m_D3DBufferLayout;
	VertexBufferLayout m_BufferLayoutArray[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

	bool m_HasLayout = false;

};

