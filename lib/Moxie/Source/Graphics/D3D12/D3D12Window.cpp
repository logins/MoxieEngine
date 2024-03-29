/*
 D3D12Window.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#include "D3D12Window.h"
#include "MoxUtils.h"
#include "D3D12UtilsInternal.h"
#include "CommandList.h"

namespace Mox
{
	D3D12Window::D3D12Window(const D3D12WindowInitInput& InInitParams)
		: m_CmdQueue(InInitParams.CmdQueue)
	{
		HINSTANCE InHInstance = ::GetModuleHandle(NULL); //Created windows will always refer to the current application instance
		m_CurrentDevice = m_CmdQueue.GetD3D12Device();

		// RTV Descriptor Size is vendor-dependent.
		// We need to retrieve it in order to know how much space to reserve per each descriptor in the Descriptor Heap
		m_RTVDescIncrementSize = m_CurrentDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		m_TearingSupported = Mox::CheckTearingSupport();
		m_VSyncEnabled = InInitParams.VSyncEnabled;
		UpdatePresentFlags();

		RegisterWindowClass(InHInstance, InInitParams.WindowClassName, InInitParams.WndProc);

		CreateHWND(InInitParams.WindowClassName, InHInstance, InInitParams.WindowTitle, InInitParams.WinWidth, InInitParams.WinHeight);

		m_RTVDescriptorHeap = Mox::CreateDescriptorHeap(m_CurrentDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_DefaultBufferCount);

		CreateSwapChain(m_CmdQueue.GetD3D12CmdQueue(), InInitParams.BufWidth, InInitParams.BufHeight);

		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		// Create Descriptor Heap for the DepthStencil View
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {}; // Note: DepthStencil View requires storage in a heap even if we are going to use only 1 view
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		Mox::ThrowIfFailed(m_CurrentDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

		UpdateBufferResourcesAndViews();

		UpdateDepthStencil();
	}

	D3D12Window::D3D12Window(const Mox::WindowInitInput& InWindowInitInput)
		: m_CmdQueue(static_cast<Mox::D3D12CommandQueue&>(InWindowInitInput.CmdQueue))
	{
		HINSTANCE InHInstance = ::GetModuleHandle(NULL); //Created windows will always refer to the current application instance
		m_CurrentDevice = m_CmdQueue.GetD3D12Device();

		m_FrameWidth = InWindowInitInput.WinWidth;
		M_FrameHeight = InWindowInitInput.WinHeight;

		// RTV Descriptor Size is vendor-dependent.
		// We need to retrieve it in order to know how much space to reserve per each descriptor in the Descriptor Heap
		m_RTVDescIncrementSize = m_CurrentDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		m_TearingSupported = Mox::CheckTearingSupport();
		m_VSyncEnabled = InWindowInitInput.vSyncEnabled;
		UpdatePresentFlags();


		RegisterWindowClass(InHInstance, InWindowInitInput.WindowClassName, nullptr);

		CreateHWND(InWindowInitInput.WindowClassName, InHInstance, InWindowInitInput.WindowTitle, InWindowInitInput.WinWidth, InWindowInitInput.WinHeight);

		m_RTVDescriptorHeap = Mox::CreateDescriptorHeap(m_CurrentDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_DefaultBufferCount);

		CreateSwapChain(m_CmdQueue.GetD3D12CmdQueue(), InWindowInitInput.BufWidth, InWindowInitInput.BufHeight);

		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		// Create Descriptor Heap for the DepthStencil View
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {}; // Note: DepthStencil View requires storage in a heap even if we are going to use only 1 view
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		Mox::ThrowIfFailed(m_CurrentDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

		UpdateBufferResourcesAndViews();

		UpdateDepthStencil();
	}

	D3D12Window::~D3D12Window() = default;


	void D3D12Window::ShowWindow()
	{
		::ShowWindow(m_HWND, SW_SHOW);
	}

	void D3D12Window::Close()
	{
		OnDestroyDelegate.Broadcast();
	}

	void D3D12Window::Present()
	{
		// If v-sync enabled we need to wait for the current (front) backbuffer to finish presenting before presenting again.
		// This happens because now it is up to the swapchain to decide at what rate to finish frames, instead of the application.
		// Swapchain base its decision on the current monitor refresh rate (vertical sync).
		if (m_VSyncEnabled)
		{
			m_CmdQueue.WaitForFenceValue(m_FrameFenceValues[m_CurrentBackBufferIndex]);
		}

		ThrowIfFailed(m_SwapChain->Present(m_VSyncEnabled, m_PresentFlags));

		// Note: the present operation changed the swapchain current backbuffer index to the next available !
		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		// Signal fence for the current backbuffer "on the fly"
		m_FrameFenceValues[m_CurrentBackBufferIndex] = m_CmdQueue.Signal();
	}

	ComPtr<ID3D12Resource> D3D12Window::GetCurrentBackbuffer()
	{
		return m_BackBuffers[m_CurrentBackBufferIndex].GetInner();
	}

	ComPtr<ID3D12Resource> D3D12Window::GetBackbufferAtIndex(uint32_t InIdx)
	{
		return InIdx < m_DefaultBufferCount ? m_BackBuffers[InIdx].GetInner() : nullptr;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12Window::GetCurrentRTVDescHandle()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentBackBufferIndex, m_RTVDescIncrementSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12Window::GetCuttentDSVDescHandle()
	{
		return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void D3D12Window::SetFullscreenState(bool InNowFullScreen)
	{
		if (m_IsFullScreen == InNowFullScreen)
			return;

		if (InNowFullScreen)
		{
			// Store previous window dimensions to be able to turn back to window mode
			::GetWindowRect(m_HWND, &m_WindowModeRect);

			UINT fullscreenStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			::SetWindowLongW(m_HWND, GWL_STYLE, fullscreenStyle);

			HMONITOR nearestMonitor = ::MonitorFromWindow(m_HWND, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX); // Fill cbSize is required to use MONITORINFOEX
			::GetMonitorInfo(nearestMonitor, &monitorInfo);

			::SetWindowPos(m_HWND, HWND_TOP, // This will place window on top of all the others
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | // Will apply the window style set with ::SetWindowLongW and send a WM_NCCALCSIZE message to the window
				SWP_NOACTIVATE // Do not trigger window activation while repositioning
			);
			::ShowWindow(m_HWND, SW_MAXIMIZE); // The actual window set fullscreen command
		}
		else // From fullscreen to window
		{
			::SetWindowLong(m_HWND, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(m_HWND, HWND_NOTOPMOST, // Generic window setting
				m_WindowModeRect.left,
				m_WindowModeRect.top,
				m_WindowModeRect.right - m_WindowModeRect.left,
				m_WindowModeRect.bottom - m_WindowModeRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);
			::ShowWindow(m_HWND, SW_NORMAL);
		}

		m_IsFullScreen = InNowFullScreen;
	}

	void D3D12Window::SetVSyncEnabled(bool InNowEnabled)
	{
		m_VSyncEnabled = InNowEnabled;
		UpdatePresentFlags();
	}

	// Note: When a window resize happens, all the swapchain backbuffers need to be released.
	// Before doing it, we need to wait for the GPU to be done with them, by flushing.
	void D3D12Window::Resize(uint32_t InNewWidth, uint32_t InNewHeight)
	{
		// Don't allow 0 size swap chain buffers
		m_FrameWidth = std::max(1u, InNewWidth);
		M_FrameHeight = std::max(1u, InNewHeight);

		// Flush GPU to ensure no commands are "in-flight" on the backbuffers
		m_CmdQueue.Flush();

		// All the backbuffers references must be released before resizing the swapchain
		for (int32_t i = 0; i < m_DefaultBufferCount; i++)
		{
			m_BackBuffers[i].GetInner().Reset();
			m_FrameFenceValues[i] = m_FrameFenceValues[m_CurrentBackBufferIndex];
		}

		// Resizing swapchain and relative backbuffers' descriptor heap
		DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
		ThrowIfFailed(m_SwapChain->GetDesc(&swapchain_desc)); // Maintain previous swapchain settings after resizing
		ThrowIfFailed(m_SwapChain->ResizeBuffers(m_DefaultBufferCount, m_FrameWidth, M_FrameHeight, swapchain_desc.BufferDesc.Format, swapchain_desc.Flags));
		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
		UpdateBufferResourcesAndViews();

		UpdateDepthStencil();

		// Note: Viewport will be updated by the Application, that in this case acts as a "frame director"
	}

	Mox::CpuDescHandle& D3D12Window::GetCurrentRTVDescriptorHandle()
	{
		// Every time these objects are constructed, maybe we can store them somewhere later on
		CurrentRtvDescHandle = Mox::D3D12CpuDescriptorHandle(GetCurrentRTVDescHandle());
		return CurrentRtvDescHandle;
	}

	Mox::CpuDescHandle& D3D12Window::GetCurrentDSVDescriptorHandle()
	{
		CurrentDsvDescHandle = Mox::D3D12CpuDescriptorHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCuttentDSVDescHandle()));
		return CurrentDsvDescHandle;
	}

	void D3D12Window::ClearRtAndDs(Mox::CommandList& InCmdList)
{
		// Transitioning current backbuffer resource to render target state
		// We can be sure that the previous state was present because in this application all the render targets
		// are first filled and then presented to the main window repetitevely.
		InCmdList.ResourceBarriers(TransitionInfoVector{ 
			{&m_BackBuffers[m_CurrentBackBufferIndex], RESOURCE_STATE::PRESENT, RESOURCE_STATE::RENDER_TARGET} 
			});

		float clearColor[] = { .4f, .6f, .9f, 1.f };
		InCmdList.ClearRTV(GetCurrentRTVDescriptorHandle(), clearColor);

		// Note: Clearing Render Target and Depth Stencil is a good practice, but in this case is also essential.
		// Without clearing the DepthStencilView, the rasterizer would not be able to use it!!
		InCmdList.ClearDepth(GetCurrentDSVDescriptorHandle());
	}

	Mox::Resource& D3D12Window::GetCurrentBackBuffer()
	{
		return m_BackBuffers[m_CurrentBackBufferIndex] ;
	}

	void D3D12Window::CreateHWND(const wchar_t* InWindowClassName, HINSTANCE InHInstance, const wchar_t* InWindowTitle, uint32_t width, uint32_t height)
	{
		int32_t screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int32_t screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
		::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		int32_t windowWidth = windowRect.right - windowRect.left;
		int32_t windowHeight = windowRect.bottom - windowRect.top;

		//Center window within the screen. Clamp to 0,0 for the top-left corner.
		//This is going to be the top-left corner
		int32_t winTLx = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int32_t winTLy = std::max<int>(0, (screenHeight - windowHeight) / 2);

		HWND currentHWND = ::CreateWindowExW(
			NULL,
			InWindowClassName,
			InWindowTitle,
			WS_OVERLAPPEDWINDOW,
			winTLx,
			winTLy,
			windowWidth,
			windowHeight,
			NULL,
			NULL,
			InHInstance,
			this
		);

		//Check for valid return value
		assert(currentHWND && "Failed to create the HWND for the current window.");

		m_HWND = currentHWND;
	}

	void D3D12Window::CreateSwapChain(ComPtr<ID3D12CommandQueue> InCmdQueue, uint32_t InBufWidth, uint32_t InBufHeight)
	{
		ComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#if _DEBUG
		createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = InBufWidth;
		swapChainDesc.Height = InBufHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Unsigned normalized integers, 8 bits per channel
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 }; // Num samples per pixel and quality
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_DefaultBufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // Stretch buffer content to match window size
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Discards previous backbuffers content
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0; // Tearing feature is required for variable refresh rate displays.
		// Allow Tearing has also to be set in Present settings.  It can only be used in windowed mode. 
		// To use this flag in full screen Win32 apps, the application should present to a fullscreen borderless window
		// and disable automatic ALT+ENTER fullscreen switching using IDXGIFactory::MakeWindowAssociation.
		// https://docs.microsoft.com/en-gb/windows/win32/direct3ddxgi/dxgi-present

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
			InCmdQueue.Get(),
			m_HWND,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		));

		// Disable Alt+Enter hotkey, necessery for allow tearing feature
		ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_HWND, DXGI_MWA_NO_ALT_ENTER));

		// Cast to swapchain 4 and assign to member variable
		ThrowIfFailed(swapChain1.As(&m_SwapChain));

		// The SwapChain will automatically allocate all the necessary back buffers, so we can now retrieve them
		m_BackBuffers.reserve(m_DefaultBufferCount);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHeapHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		Microsoft::WRL::ComPtr<ID3D12Resource> tempBB;
		for (int bufferIdx = 0; bufferIdx < m_DefaultBufferCount; bufferIdx++)
		{
			// Retrieve interface to the current backBuffer
			ThrowIfFailed(m_SwapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&tempBB)));

			m_CurrentDevice->CreateRenderTargetView(tempBB.Get(), nullptr, rtvDescHeapHandle);

			m_BackBuffers.emplace_back(tempBB, Mox::D3D12_RES_TYPE::BackBuffer);

			rtvDescHeapHandle.Offset(m_RTVDescIncrementSize); // Shift rtvDescHandle pointer to the next element in desc heap
		}

	}

	void D3D12Window::UpdateDepthStencil()
	{
		// Allocate DepthStencil resource in GPU
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.f, 0 }; 
		// Note: passing the same address of ID3DResource of m_DSBuffer to CreateDepthStencilCommittedResource on a second call of this function
		// will actually replace (update) the committed resource on the GPU (and so it will recreate the corresponding heap)

		m_DSBuffer.Reset(); // Note: Need to release the interface to the previous depth-stencil buffer before assigning to a new one or there will be an interface leak!

		Mox::CreateDepthStencilCommittedResource(m_CurrentDevice, m_DSBuffer.GetAddressOf(), m_FrameWidth, M_FrameHeight,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue);
		// Need to update the view pointing to the updated resource
		Mox::CreateDepthStencilView(m_CurrentDevice, m_DSBuffer.Get(), m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void D3D12Window::UpdateBufferResourcesAndViews()
	{
		//Getting a descriptor handle to iterate trough the descriptor heap elements
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHeapHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT bufferIdx = 0; bufferIdx < m_DefaultBufferCount; bufferIdx++)
		{
			ComPtr<ID3D12Resource> backBuffer;
			// Retrieve interface to the current backBuffer
			ThrowIfFailed(m_SwapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&backBuffer)));
			// Create the RTV and store the reference of the current backBuffer, which is where rtvDescHeapHandle is pointing to
			// Either the pointer to the resource or the resource descriptor must be provided to create the view.
			m_CurrentDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvDescHeapHandle);

			m_BackBuffers[bufferIdx].SetInner(backBuffer);

			// Move the CPU descriptor handle to the next element on the heap
			rtvDescHeapHandle.Offset(m_RTVDescIncrementSize);
		}
	}

	LRESULT D3D12Window::GlobalWndProc(HWND InHwnd, UINT InMsg, WPARAM InWParam, LPARAM InLParam)
	{
		if(InMsg == WM_NCCREATE)
			SetWindowLongPtr(InHwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)InLParam)->lpCreateParams);
				
		D3D12Window* wndptr = (D3D12Window*) ::GetWindowLongPtr(InHwnd, GWLP_USERDATA);

		if (wndptr)
		{
			D3D12Window& currentWindow = *wndptr;
			switch (InMsg)
			{
			case WM_CREATE:
				currentWindow.OnCreateDelegate.Broadcast();
				break;
			case WM_PAINT:
				currentWindow.OnPaintDelegate.Broadcast();
				break;
			case  WM_KEYDOWN:
			{
				switch (InWParam) // Only two cases as an example, easily extensible
				{
				case  'V':
					currentWindow.OnTypingKeyDownDelegate.Broadcast(Mox::KEYBOARD_KEY::KEY_V);
					break;
				case  VK_ESCAPE:
					currentWindow.OnControlKeyDownDelegate.Broadcast(Mox::KEYBOARD_KEY::KEY_ESC);
					break;
				}
				break;
			}
			case WM_MOUSEMOVE:
				currentWindow.OnMouseMoveDelegate.Broadcast(GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				break;
			case WM_SYSCHAR:
				assert(false); // TODO do we need this at all?
				break; // Preventing Alt+Enter hotkey to try switching the window to fullscreen since we are manually handling it with Alt+F11
			case WM_MOUSEWHEEL:
				currentWindow.OnMouseWheelDelegate.Broadcast(static_cast<float>(GET_WHEEL_DELTA_WPARAM(InWParam)));
				break;
			case WM_LBUTTONDOWN:
				{
					POINT clickedPoint = { GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam) };
					currentWindow.m_IsMouseLeftHold = static_cast<bool>(::DragDetect(InHwnd, clickedPoint)); // Note: DragDetect will activate only if the mouse is dragged for some pixels, but it only works with the left mouse button!
					currentWindow.OnMouseButtonDownDelegate.Broadcast(0, clickedPoint.x, clickedPoint.y);
					::SetCapture(InHwnd); // This will allow to track drag operation when mouse position exits the window boundaries
					break;
				}
			case WM_MBUTTONDOWN:
				currentWindow.OnMouseButtonDownDelegate.Broadcast(1, GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				break;
			case WM_RBUTTONDOWN:
			{
				currentWindow.m_IsMouseRightHold = true; // Note: We cannot use DragDetect with something different than the left mouse button, so we just set right mouse hold every time we click with the right button, which is not perfect, but for this usage, it will do.
				currentWindow.OnMouseButtonDownDelegate.Broadcast(2, GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				::SetCapture(InHwnd); // This will allow to track drag operation when mouse position exits the window boundaries
				break;
			}
			case WM_LBUTTONUP:
			{
				currentWindow.m_IsMouseLeftHold = false;
				currentWindow.OnMouseButtonUpDelegate.Broadcast(0, GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				::ReleaseCapture(); // Correspondent of SetCapture
				break;
			}
			case WM_MBUTTONUP:
				currentWindow.OnMouseButtonUpDelegate.Broadcast(1, GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				break;
			case WM_RBUTTONUP:
			{
				currentWindow.m_IsMouseRightHold = false;
				currentWindow.OnMouseButtonUpDelegate.Broadcast(2, GET_X_LPARAM(InLParam), GET_Y_LPARAM(InLParam));
				::ReleaseCapture(); // Correspondent of SetCapture
				break;
			}
			case WM_SIZE:
			{
				RECT clientRect = {};
				::GetClientRect(InHwnd, &clientRect);
				const int32_t newWidth = clientRect.right - clientRect.left;
				const int32_t newHeight = clientRect.bottom - clientRect.top;
				currentWindow.Resize(newWidth, newHeight);
				currentWindow.OnResizeDelegate.Broadcast(newWidth, newHeight);
				break;
			}
			case WM_DESTROY:
				currentWindow.Close();
				break;
			}
		}

		return ::DefWindowProcW(InHwnd, InMsg, InWParam, InLParam); //Message will be handled by the Default Window Procedure !
	}

	void D3D12Window::UpdatePresentFlags()
	{
		m_PresentFlags = m_TearingSupported && !IsVSyncEnabled() ? DXGI_PRESENT_ALLOW_TEARING : 0;
	}

	void D3D12Window::RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName, WNDPROC InWndProc)
	{
		// Creating a window class to represent out render window
		WNDCLASSEXW windowClass = {};

		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW; // Redraw window if movement or size adjustments horizontally or vertically
		windowClass.lpfnWndProc = InWndProc? InWndProc : &D3D12Window::GlobalWndProc;
		windowClass.cbClsExtra = 0; // Extra bytes to allocate after window class
		windowClass.cbWndExtra = 0; // Extra bytes to allocate after window instance
		windowClass.hInstance = hInst;
		windowClass.hIcon = ::LoadIcon(hInst, NULL); // Default icon
		windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // +1 must be added by documentation
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = windowClassName;
		windowClass.hIconSm = ::LoadIcon(hInst, NULL);

		static ATOM atom = ::RegisterClassExW(&windowClass);
		assert(atom > 0);
	}
}