#include "pch.h"
#include "DX11Shader.h"

DX11Shader::DX11Shader(DX11Context& context, const std::string& filepath, Shader::ShaderType type)
	: m_DX11Context(context), m_Filepath(filepath), m_ShaderType(type)
{
	if (m_Filepath != "")
	{
		DWORD shaderFlags = 0;
#ifdef DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		std::wstring filePathWide = std::wstring(m_Filepath.begin(), m_Filepath.end());
		LPCWSTR filePathWideCString = filePathWide.c_str();


		HRESULT hr = D3DCompileFromFile(
			filePathWideCString,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",
			ShaderTypeToCompilerTarget().c_str(),
			shaderFlags,
			0,
			&m_CompiledShader,
			&m_CompilationErrorMsgs
		);

		if (m_CompilationErrorMsgs != 0)
		{
			LOG_ERROR("SHADER COMPILATION ERROR: {0}", static_cast<char*>(m_CompilationErrorMsgs->GetBufferPointer()));
		}

		ASSERT_HR(hr);

		switch (m_ShaderType)
		{
		case VERTEX_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreateVertexShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_VertexShader));
			break;
		case PIXEL_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreatePixelShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_PixelShader));
			break;
		case COMPUTE_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreateComputeShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_ComputeShader));
			break;
		case GEOMETRY_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreateGeometryShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_GeometryShader));
			break;
		case HULL_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreateHullShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_HullShader));
			break;
		case DOMAIN_SHADER:
			ASSERT_HR(m_DX11Context.GetDevice().CreateDomainShader(m_CompiledShader->GetBufferPointer(), m_CompiledShader->GetBufferSize(), nullptr, &m_DomainShader));
			break;
		default:
			ASSERT(false, "Unknown shader type");

		}
	}
}

void DX11Shader::Bind() const
{
	if (m_Filepath != "")
	{

		switch (m_ShaderType)
		{
		case VERTEX_SHADER:
			m_DX11Context.GetDeviceContext().VSSetShader(m_VertexShader.Get(), nullptr, 0);
			break;
		case PIXEL_SHADER:
			m_DX11Context.GetDeviceContext().PSSetShader(m_PixelShader.Get(), nullptr, 0);
			break;
		case COMPUTE_SHADER:
			m_DX11Context.GetDeviceContext().CSSetShader(m_ComputeShader.Get(), nullptr, 0);
			break;
		case GEOMETRY_SHADER:
			m_DX11Context.GetDeviceContext().GSSetShader(m_GeometryShader.Get(), nullptr, 0);
			break;
		case HULL_SHADER:
			m_DX11Context.GetDeviceContext().HSSetShader(m_HullShader.Get(), nullptr, 0);
			break;
		case DOMAIN_SHADER:
			m_DX11Context.GetDeviceContext().DSSetShader(m_DomainShader.Get(), nullptr, 0);
			break;
		default:
			ASSERT(false, "Unknown shader type");
		}
	}
	else
	{
		switch (m_ShaderType)
		{
		case VERTEX_SHADER:
			m_DX11Context.GetDeviceContext().VSSetShader(nullptr, nullptr, 0);
			break;
		case PIXEL_SHADER:
			m_DX11Context.GetDeviceContext().PSSetShader(nullptr, nullptr, 0);
			break;
		case COMPUTE_SHADER:
			m_DX11Context.GetDeviceContext().CSSetShader(nullptr, nullptr, 0);
			break;
		case GEOMETRY_SHADER:
			m_DX11Context.GetDeviceContext().GSSetShader(nullptr, nullptr, 0);
			break;
		case HULL_SHADER:
			m_DX11Context.GetDeviceContext().HSSetShader(nullptr, nullptr, 0);
			break;
		case DOMAIN_SHADER:
			m_DX11Context.GetDeviceContext().DSSetShader(nullptr, nullptr, 0);
			break;
		default:
			ASSERT(false, "Unknown shader type");
		}
	}
}

const std::string DX11Shader::ShaderTypeToCompilerTarget()
{
	switch (m_ShaderType)
	{
	case VERTEX_SHADER:
		return "vs_5_0";
	case PIXEL_SHADER:
		return "ps_5_0";
	case COMPUTE_SHADER:
		return "cs_5_0";
	case GEOMETRY_SHADER:
		return "gs_5_0";
	case HULL_SHADER:
		return "hs_5_0";
	case DOMAIN_SHADER:
		return "ds_5_0";
	default:
		ASSERT(false, "Unknown shader type");
		return "";
	}

}
