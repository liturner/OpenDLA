#include "Renderer.h"

OpenDLA::Renderer::Renderer() :
	m_pDevice(NULL),
	m_pDeviceContext(NULL),
	m_pD3dDebug(NULL),
	m_pDXGISwapChain(NULL),
	m_pBackBuffer(NULL),
	m_pDepthStencil(NULL),
	m_pRenderTarget(NULL),
	m_pDepthStencilView(NULL),
	m_pVertexShader(NULL),
	m_pPixelShader(NULL),
	m_pInputLayout(NULL),
	m_pVertexBuffer(NULL)
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

	m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_pD3dDebug));
	
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

HRESULT OpenDLA::Renderer::OnWindowResize(const RECT& _windowRect)
{
	// Small safety check to make sure we dont do anything stupid here
	if (!m_pDevice) return FALSE;

	SAFE_RELEASE(m_pWindowDependantCBuffer)

	// https://en.wikipedia.org/wiki/Orthographic_projection
	ScreenDependantConstantBuffer m_constantBufferData;
	m_constantBufferData.orthoOriginZProjection = DirectX::XMFLOAT4X4(2.f / (float)_windowRect.right, 0.f, 0.f, 0.f,
		0.f, 2.f / (float)_windowRect.top, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	CD3D11_BUFFER_DESC cbDesc(
		sizeof(ScreenDependantConstantBuffer),
		D3D11_BIND_CONSTANT_BUFFER
	);

	HRESULT hr = m_pDevice->CreateBuffer(
		&cbDesc,
		nullptr,
		&m_pWindowDependantCBuffer
	);

	m_pDeviceContext->UpdateSubresource(m_pWindowDependantCBuffer, 0, nullptr, &m_constantBufferData, 0, 0);

	// Vertex Buffer with a dummy Point for now
	Point* temp = new Point[1920 * 1080];

	CD3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(CD3D11_BUFFER_DESC));
	vDesc.Usage = D3D11_USAGE_DYNAMIC;
	vDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(*temp) * 1920 * 1080; // Cannot cheaply resize later  so just allocate a load now

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = temp;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	hr = m_pDevice->CreateBuffer(&vDesc, &vData, &m_pVertexBuffer);

	delete[] temp;

	return hr;
}

void OpenDLA::Renderer::Render(DLASimulation _simulation)
{
	// Vertex Buffer
	// m_pDeviceContext->Map()
	D3D11_MAPPED_SUBRESOURCE resource;
	m_pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &_simulation.m_points[0], sizeof(_simulation.m_points[0]) * _simulation.m_points.size());
	m_pDeviceContext->Unmap(m_pVertexBuffer, 0);

	///
	/// DirectX Pipeline
	///

	// Clear the screen
	const float teal[] = { 0.098f, 0.439f, 0.439f, 1.000f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTarget, teal);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,	0);

	// Input Assembly Stage
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	const UINT stride = sizeof(Point);
	const UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0,	1, &m_pVertexBuffer, &stride, &offset);

	// Vertex Shader Stage
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pWindowDependantCBuffer);

	// Set the render target.
	m_pDeviceContext->OMSetRenderTargets(1,	&m_pRenderTarget, m_pDepthStencilView);
	m_pDeviceContext->Draw(1,0);
}

void OpenDLA::Renderer::Present()
{
	m_pDXGISwapChain->Present(1, 0);
}

void OpenDLA::Renderer::Release()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();

	if(m_pDXGISwapChain) m_pDXGISwapChain->Release();

	if(m_pBackBuffer) m_pBackBuffer->Release();
	if(m_pDepthStencil) m_pDepthStencil->Release();
	if(m_pRenderTarget) m_pRenderTarget->Release();;
	if(m_pDepthStencilView) m_pDepthStencilView->Release();
	if(m_pDeviceContext) m_pDeviceContext->Release();
	if(m_pDevice) m_pDevice->Release();
	
	if(m_pD3dDebug)
		m_pD3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
}

HRESULT OpenDLA::Renderer::LoadShaders()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);

	// Vertex Shader
	ID3DBlob* vsBlob = nullptr;
	HRESULT hr = CompileShader(L"VertexShader.hlsl", "main", "vs_4_0_level_9_1", &vsBlob);
	if (FAILED(hr)) 
		return hr;
	hr = m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		vsBlob->Release();
		return hr;
	}

	// Vertex Description
	D3D11_INPUT_ELEMENT_DESC iaDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = m_pDevice->CreateInputLayout(iaDesc, ARRAYSIZE(iaDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		vsBlob->Release();
		return hr;
	}

	vsBlob->Release();
	m_pDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);

	// Pixel Shader
	ID3DBlob* psBlob = nullptr;
	hr = CompileShader(L"PixelShader.hlsl", "main", "ps_4_0_level_9_1", &psBlob);
	if (FAILED(hr))
		return hr;
	hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &m_pPixelShader);
	if (FAILED(hr))
	{
		psBlob->Release();
		return hr;
	}
	psBlob->Release();
	m_pDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);

	return hr;
}


HRESULT OpenDLA::Renderer::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob * *blob)
{
	if (!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}
