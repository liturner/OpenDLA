#pragma once
#include "framework.h"
#include <d3d11.h>
#include <d3d11_2.h>

namespace OpenDLA
{
	class Renderer
	{
	public:
		Renderer();

		HRESULT Initialise(HWND hWnd);
		void Render();
		void Present();
		void Release();

	private:
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pDXGISwapChain;

		ID3D11Texture2D* m_pBackBuffer;
		ID3D11Texture2D* m_pDepthStencil;
		ID3D11RenderTargetView* m_pRenderTarget;
		ID3D11DepthStencilView* m_pDepthStencilView;
		
		D3D11_TEXTURE2D_DESC m_bbDesc;
		D3D11_VIEWPORT m_viewport;

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Device* get_pDevice() { return m_pDevice; };

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Texture2D* get_pBackBuffer() { return m_pBackBuffer; };

		// We are protecting initialisation by exiting on failure, so cannot be null
		_Notnull_ ID3D11Texture2D* get_pDepthStencil() { return m_pDepthStencil; };
	};
}