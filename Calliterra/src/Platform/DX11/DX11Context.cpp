#include "pch.h"

#include "Core/Timer.h"
#include "DX11Context.h"
#include <dxgi1_2.h>

#include <backends/imgui_impl_dx11.cpp>

#include "Platform/DX11/DX11RenderTarget.h"

DX11Context::DX11Context(HWND* hWnd, WindowProps& windowProps)
	: m_Hwnd(hWnd), m_WindowProps(windowProps)
{
	ASSERT(hWnd, "Window handle is null!");
}

void DX11Context::Init()
{
	CreateDeviceContext();
	CreateSwapChain();
	CreateRenderTargetView();
	CreateDepthStencilBuffer();
	SetRenderViewport(0.0f, 0.0f, static_cast<float>(m_WindowProps.Width), static_cast<float>(m_WindowProps.Height));

	m_OutputDevices = DX11OutputDevices(m_Device);

	ImGui_ImplDX11_Init(m_Device.Get(), m_DeviceContext.Get());
}

void DX11Context::SwapBuffers()
{
	ASSERT_HR(m_SwapChain->Present(m_VSyncEnabled, 0));
}

void DX11Context::DrawIndexed(uint32_t indexCount)
{

	m_DeviceContext->DrawIndexed(indexCount, 0, 0);
}

std::shared_ptr<RenderTarget> DX11Context::GetBackBufferTarget() const
{
	return m_RenderTarget;
}

uint32_t DX11Context::GetWidth() const
{
	return m_WindowProps.Width;
}

uint32_t DX11Context::GetHeight() const
{
	return m_WindowProps.Height;
}

void DX11Context::CreateDeviceContext()
{
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_1 };
	UINT feautreLevelCount = sizeof(featureLevelArray) / sizeof(D3D_FEATURE_LEVEL);

	UINT createDeviceFlags = 0;
#ifdef DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ASSERT_HR(
		D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevelArray,
			feautreLevelCount,
			D3D11_SDK_VERSION,
			&m_Device,
			&featureLevel,
			&m_DeviceContext
		)
	);

	if(featureLevel == D3D_FEATURE_LEVEL_11_1)
		LOG_INFO("DirectX 11.1 Context Initialized");

#ifdef DEBUG
	// Set up debug layer to break on D3D11 errors
	ComPtr<ID3D11Debug> d3dDebug;
	ASSERT_HR(
		m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)
	);

	ComPtr<ID3D11InfoQueue> d3dInfoQueue;
	ASSERT_HR(
		d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)
	);

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif

}

void DX11Context::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGIDevice2> dxgiDevice;
	ASSERT_HR(m_Device->QueryInterface(__uuidof(IDXGIDevice2), (void**)&dxgiDevice));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ASSERT_HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	ComPtr<IDXGIFactory2> dxgiFactory;
	ASSERT_HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));

	ASSERT_HR(dxgiFactory->CreateSwapChainForHwnd(
		m_Device.Get(),
		*m_Hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&m_SwapChain
		)
	);

	ASSERT_HR(dxgiFactory->MakeWindowAssociation(
		*m_Hwnd,
		DXGI_MWA_NO_ALT_ENTER
		)
	);

}

void DX11Context::CreateRenderTargetView()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);

	m_RenderTarget = std::shared_ptr<DX11OutputOnlyRenderTarget>(new DX11OutputOnlyRenderTarget(*this, backBuffer.Get()));
}

void DX11Context::CreateDepthStencilBuffer()
{
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = m_WindowProps.Width;
	depthStencilDesc.Height = m_WindowProps.Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ASSERT_HR(m_Device->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBuffer));

	D3D11_DEPTH_STENCIL_VIEW_DESC  descDSV = {};
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	ASSERT_HR(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &descDSV, &m_DepthStencilView));

	m_DeviceContext->OMSetRenderTargets(1, m_RenderTarget->GetRenderTargetView().GetAddressOf(), m_DepthStencilView.Get());
}

void DX11Context::SetRenderViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewportDesc = {};
	viewportDesc.TopLeftX = x;
	viewportDesc.TopLeftY = y;
	viewportDesc.Width = width;
	viewportDesc.Height = height;
	viewportDesc.MinDepth = 0.0f;
	viewportDesc.MaxDepth = 1.0f;

	m_DeviceContext->RSSetViewports(1, &viewportDesc);
}

void DX11Context::OnWindowResize() 
{
	ASSERT(m_DeviceContext);
	ASSERT(m_Device);
	ASSERT(m_SwapChain);

	m_RenderTarget.reset();
	m_DepthStencilView.Reset();
	m_DepthStencilBuffer.Reset();

	m_SwapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	CreateRenderTargetView();
	CreateDepthStencilBuffer();

	SetRenderViewport(0.0f, 0.0f, static_cast<float>(m_WindowProps.Width), static_cast<float>(m_WindowProps.Height));
}

void DX11Context::ToggleFullscreen()
{
	BOOL fullscreen;
	ASSERT_HR(m_SwapChain->GetFullscreenState(&fullscreen, nullptr));

	if (fullscreen)
	{
		ASSERT_HR(m_SwapChain->SetFullscreenState(FALSE, nullptr));
	}
	else
	{

		DX11OutputDevice* monitor = m_OutputDevices.GetCurrentWindowOutputDevice(*m_Hwnd);
		ComPtr<IDXGIOutput> device = monitor->GetDevice();

		DXGI_MODE_DESC modeDesc = {};
		RECT desktopArea = monitor->GetDesktopArea();

		modeDesc.Width = desktopArea.right - desktopArea.left;
		modeDesc.Height = desktopArea.bottom - desktopArea.top;

		ASSERT_HR(m_SwapChain->ResizeTarget(&modeDesc));
	
		ASSERT_HR(m_SwapChain->SetFullscreenState(TRUE, device.Get()));

		ASSERT_HR(m_SwapChain->ResizeTarget(&modeDesc));

		//m_RenderTargetView.Reset();
		m_RenderTarget.reset();
		m_DepthStencilView.Reset();
		m_DepthStencilBuffer.Reset();
		ASSERT_HR(m_SwapChain->ResizeBuffers(
			0, // 0 preserves the existing number of buffers
			0,
			0,
			DXGI_FORMAT_UNKNOWN, // Preserves the existing format
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			)
		);

		CreateRenderTargetView();
		CreateDepthStencilBuffer();

		SetRenderViewport(0.0f, 0.0f, static_cast<float>(m_WindowProps.Width), static_cast<float>(m_WindowProps.Height));
	}
}
