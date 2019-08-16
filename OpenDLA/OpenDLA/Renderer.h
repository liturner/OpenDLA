#pragma once
#include "framework.h"
#include <d3d11.h>
#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = NULL; } }

namespace OpenDLA
{

	struct Point
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	// Struct holds only values which need to be updated when the Window changes size
	struct ScreenDependantConstantBuffer {
		DirectX::XMFLOAT4X4 orthoOriginZProjection;
	};

	class Renderer
	{
	public:
		Renderer();

		HRESULT Initialise(HWND hWnd);
		HRESULT OnWindowResize(const RECT& _windowRect);
		void Render();
		void Present();
		void Release();

		// Helper for reloading all Shaders
		HRESULT LoadShaders();

	private:
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		ID3D11Debug* m_pD3dDebug;
		IDXGISwapChain* m_pDXGISwapChain;

		ID3D11Texture2D* m_pBackBuffer;
		ID3D11Texture2D* m_pDepthStencil;
		ID3D11RenderTargetView* m_pRenderTarget;
		ID3D11DepthStencilView* m_pDepthStencilView;
		
		ID3D11VertexShader* m_pVertexShader;
		ID3D11PixelShader* m_pPixelShader;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;

		ID3D11Buffer* m_pWindowDependantCBuffer;

		D3D11_TEXTURE2D_DESC m_bbDesc;
		D3D11_VIEWPORT m_viewport;

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Device* get_pDevice() { return m_pDevice; };

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Texture2D* get_pBackBuffer() { return m_pBackBuffer; };

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Texture2D* get_pDepthStencil() { return m_pDepthStencil; };

		// Internal call for compiling Shaders
		HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);
	};
}