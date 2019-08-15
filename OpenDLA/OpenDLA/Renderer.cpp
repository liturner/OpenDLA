#include "Renderer.h"

OpenDLA::Renderer::Renderer() :
	m_pDevice(NULL),
	m_pDeviceContext(NULL),
	m_pDXGISwapChain(NULL),
	m_pBackBuffer(NULL),
	m_pDepthStencil(NULL),
	m_pRenderTarget(NULL),
	m_pDepthStencilView(NULL)
{
	m_bbDesc = D3D11_TEXTURE2D_DESC();
	m_viewport = D3D11_VIEWPORT();
}

HRESULT OpenDLA::Renderer::Initialise(HWND hWnd)
{
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

	// This flag adds support for surfaces with a color-channel ordering different
	// from the API default. It is required for compatibility with Direct2D.
	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create the Direct3D 11 API device object and a corresponding context.
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		deviceFlags,                // Set debug and Direct2D compatibility flags.
		levels,                     // List of feature levels this app can support.
		ARRAYSIZE(levels),          // Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&m_pDevice,                    // Returns the Direct3D device created.
		&featureLevel,            // Returns feature level of device created.
		&m_pDeviceContext                    // Returns the device immediate context.
	);

	// Ensure success of pDevice creation, or bail as we rely on pDevice
	if (FAILED(hr))
		return hr;

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
	desc.BufferCount = 2;
	desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SampleDesc.Count = 1;      //multisampling setting
	desc.SampleDesc.Quality = 0;    //vendor-specific flag
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.OutputWindow = hWnd;

	// Create the DXGI device object to use in other factories, such as Direct2D.
	IDXGIDevice* pdxgiDevice;

	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)& pdxgiDevice);

	// Create swap chain.
	IDXGIAdapter* adapter;
	IDXGIFactory* factory;
	
	hr = pdxgiDevice->GetAdapter(&adapter);

	if (SUCCEEDED(hr))
	{
		adapter->GetParent(IID_PPV_ARGS(&factory));
		hr = factory->CreateSwapChain(get_pDevice(), &desc, &m_pDXGISwapChain);
	}

	// Back Buffer
	hr = m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)& m_pBackBuffer);
	if (FAILED(hr))
		return hr;
	
	hr = m_pDevice->CreateRenderTargetView(get_pBackBuffer(), nullptr,	&m_pRenderTarget);
	if (FAILED(hr))
		return hr;

	m_pBackBuffer->GetDesc(&m_bbDesc);

	// Depth Stencil
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, static_cast<UINT> (m_bbDesc.Width), static_cast<UINT> (m_bbDesc.Height), 1, 1, D3D11_BIND_DEPTH_STENCIL);
	m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencil);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	m_pDevice->CreateDepthStencilView(get_pDepthStencil(), &depthStencilViewDesc, &m_pDepthStencilView);

	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.Height = (float)m_bbDesc.Height;
	m_viewport.Width = (float)m_bbDesc.Width;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	m_pDeviceContext->RSSetViewports(1, &m_viewport);

	return TRUE;
}

void OpenDLA::Renderer::Render()
{
	const float teal[] = { 0.098f, 0.439f, 0.439f, 1.000f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTarget, teal);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,	0);

	// Set the render target.
	m_pDeviceContext->OMSetRenderTargets(1,	&m_pRenderTarget, m_pDepthStencilView);
	m_pDeviceContext->DrawAuto();
}

void OpenDLA::Renderer::Present()
{
	m_pDXGISwapChain->Present(0, 0);
}

void OpenDLA::Renderer::Release()
{
	if(m_pDevice) m_pDevice->Release();
	if(m_pDeviceContext) m_pDeviceContext->Release();
	if(m_pDXGISwapChain) m_pDXGISwapChain->Release();

	if(m_pBackBuffer) m_pBackBuffer->Release();
	if(m_pDepthStencil) m_pDepthStencil->Release();
	if(m_pRenderTarget) m_pRenderTarget->Release();;
	if(m_pDepthStencilView) m_pDepthStencilView->Release();
}
