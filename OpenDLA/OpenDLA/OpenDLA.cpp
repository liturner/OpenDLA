// OpenDLA.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "resource.h"
#include "Renderer.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

OpenDLA::Renderer renderer;

// Frame Time Variables
std::chrono::time_point<std::chrono::system_clock> g_frameStart;
std::chrono::time_point<std::chrono::system_clock> g_frameEnd;
double g_frameTime;

// Main Window
RECT mainWindowRect;
HWND hWnd;
RECT d3DWindowRect;
HWND d3DWindowHWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
namespace OpenDLA {
	namespace Simulation {
		void				Destroy();
		void				OnStart();
		DirectX::XMFLOAT3	OnStep(const Point& _point);
		bool				Collides(const Point& _point);
	}
}

namespace OpenDLA {
	namespace Simulation {
		PyObject* m_pModule = nullptr;
		PyObject* m_pStepFun = nullptr;
		PyObject* m_pOnStartFun = nullptr;
		std::vector<Point> m_points;
		std::vector<unsigned int> m_walkers;

		static PyObject* AddPoint(PyObject* self, PyObject* args)
		{
			Point newPoint = Point();

			//DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3();
			DirectX::XMFLOAT3 col = DirectX::XMFLOAT3();

			if (!PyArg_ParseTuple(args, "(fff)|(fff)", &newPoint.pos.x, &newPoint.pos.y, &newPoint.pos.z, &newPoint.color.x, &newPoint.color.y, &newPoint.color.z))
			{
				PyErr_Print();
				Py_RETURN_FALSE;
			}

			m_points.push_back(newPoint);

			Py_RETURN_NONE;
		}

		static PyMethodDef OpenDLA_PyMethodDef[] = {
			{"addPoint", AddPoint, METH_VARARGS, "Add a point to the simulation."},
			{NULL, NULL, 0, NULL}
		};

		static PyModuleDef OpenDLA_PyModuleDef = {
			PyModuleDef_HEAD_INIT, "OpenDLA", NULL, -1, OpenDLA_PyMethodDef,
			NULL, NULL, NULL, NULL
		};

		static PyObject* PyInit_OpenDLA(void)
		{
			return PyModule_Create(&OpenDLA_PyModuleDef);
		}

		bool Initialise()
		{
			PyImport_AppendInittab("OpenDLA", &PyInit_OpenDLA);
			Py_Initialize();
			PyObject* pName = PyUnicode_DecodeFSDefault("simulation");
			m_pModule = PyImport_Import(pName);
			Py_DECREF(pName);
			if (!m_pModule) return false;

			// It is not a failure if the function is not there, but it is a fail
			// if the function is there but not callable
			m_pStepFun = PyObject_GetAttrString(m_pModule, "OnStep");
			if (m_pStepFun && !PyCallable_Check(m_pStepFun))
			{
				Destroy();
				return false;
			}

			m_pOnStartFun = PyObject_GetAttrString(m_pModule, "OnStart");
			if (m_pOnStartFun && !PyCallable_Check(m_pOnStartFun))
			{
				Destroy();
				return false;
			}

			// Finally
			OnStart();

			return true;
		}

		void Update()
		{
			/*
			Logic:
			1. Move the walker
			2. Check for Collision
			3. Save walker as point if needed
			4. Spawn new Walker
			*/

			// 1
			OnStep(m_points[m_walkers[0]]);
			DirectX::XMFLOAT3 move = OnStep(m_points[m_walkers[0]]);
			m_points[m_walkers[0]].pos.x = m_points[m_walkers[0]].pos.x + move.x;
			m_points[m_walkers[0]].pos.y = m_points[m_walkers[0]].pos.y + move.y;
			m_points[m_walkers[0]].pos.z = m_points[m_walkers[0]].pos.z + move.z;

			// 2
			if (Collides(m_points[m_walkers[0]]))
			{
				m_points.push_back(m_points[m_walkers[0]]);
				m_walkers[0]++;
			}

		}

		void Destroy()
		{
			if (m_pStepFun) Py_DECREF(m_pStepFun);
			if (m_pOnStartFun) Py_DECREF(m_pOnStartFun);
			Py_DECREF(m_pModule);
		}

		bool Collides(const Point& _point)
		{
			return false;
		}

		DirectX::XMFLOAT3 OnStep(const Point& _point)
		{
			PyObject* returned = PyObject_CallObject(m_pStepFun, nullptr);
			DirectX::XMFLOAT3 ret(0, 0, 0);

			if (!PyArg_ParseTuple(returned, "ff|f", &ret.x, &ret.y, &ret.z))
			{
				PyErr_Print();
				return ret;
			}

			if (returned)
				Py_DECREF(returned);

			return ret;
		}

		void OnStart()
		{
			if (m_pOnStartFun != nullptr)
			{
				// It is up to the user to define the simulation logic. They may not
				// define a seed, but may investigate two walkers colliding with one 
				// another
				PyObject_CallObject(m_pOnStartFun, nullptr);

				m_walkers.push_back(1);
			}
			else
			{
				// If the user has not defined an OnStart function, we just initialise
				// to a single [0 0 0] seed.
				Point a;
				a.pos = DirectX::XMFLOAT3(0, 0, 0);
				a.color = DirectX::XMFLOAT3(1, 0, 0);

				m_points.push_back(a);

				a.pos = DirectX::XMFLOAT3(10, 0, 0);
				a.color = DirectX::XMFLOAT3(0, 1, 1);

				m_points.push_back(a);

				m_walkers.push_back(1);
			}
		}
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OPENDLA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	d3DWindowHWnd = CreateWindowW(szWindowClass, szTitle, WS_CHILD, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);	// WS_CHILD is important for Child Windows!. Pos / Size is ignored as updated with WM_SIZE
	SetParent(d3DWindowHWnd, hWnd);	// Without this, the window can be moved like an entirely seperate window

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	ShowWindow(d3DWindowHWnd, nCmdShow);
	//UpdateWindow(hWndD3D);

	// Renderer Initial Setup
	HRESULT hr = renderer.Initialise(d3DWindowHWnd);
	if (FAILED(hr))
		return hr;

	hr = renderer.LoadShaders();
	if (FAILED(hr))
		return hr;

	hr = renderer.OnWindowResize(d3DWindowRect);
	if (FAILED(hr))
		return hr;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OPENDLA));

	bool bQuit = false;
	bool bGotMsg;
    MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	
	// Handle the Python Logic initialisation
	if (!OpenDLA::Simulation::Initialise())
		return 1;

	

    // Main message loop:
	while (!bQuit)
	{
		g_frameStart = std::chrono::system_clock::now();
		
		// Process window events.
		// Use PeekMessage() so we can use idle time to render the scene. 
		bGotMsg = (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			if (msg.message == WM_QUIT)
				bQuit = true;

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				// Translate and dispatch the message
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// Update the scene.
			OpenDLA::Simulation::Update();
			// renderer.Update();

			// Render frames during idle time (when no messages are waiting).
			renderer.Render(OpenDLA::Simulation::m_points);

			// Present the frame to the screen.
			renderer.Present();

			// We should only get here after all windows events are done too thanks to bGotMsg
			g_frameEnd = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = g_frameEnd - g_frameStart;
			g_frameTime = elapsed_seconds.count();

			// If needed, cap to 30Fps
			double targetMinimumFrameTime = 1.0 / 30.0;
			if (g_frameTime < targetMinimumFrameTime)
			{
				DWORD sleepFor = DWORD((targetMinimumFrameTime - g_frameTime) * 1000.0);
				Sleep(sleepFor);
			}
		}
	}

	// Do Cleanup here

	// ToDo: Cleanup
	renderer.Release();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OPENDLA));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_OPENDLA);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


void CreateControls(HWND hWnd)
{
	CreateWindowW(L"static", L"Please Work!", WS_VISIBLE | WS_CHILD , 50, 50, 200, 50, hWnd, NULL, NULL, NULL);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the window. Be aware that the child d3d window will also send messages here
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// We can just ignore the D3D Window Messages for now
	// ToDo: Check if I can register a window without a WndProc
	if (hWnd == d3DWindowHWnd)
		return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDM_RELOADSHADER:
				renderer.LoadShaders();
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_CREATE:
		CreateControls(hWnd);
		break;
	case WM_SIZE:
		// Update the global size variables and the d3d child window size
		GetClientRect(hWnd, &mainWindowRect);
		d3DWindowRect = mainWindowRect;
		d3DWindowRect.left = 200;
		MoveWindow(d3DWindowHWnd, d3DWindowRect.left, d3DWindowRect.top, d3DWindowRect.right - d3DWindowRect.left, d3DWindowRect.bottom, TRUE);

		// Reuse the d3DWindowRect but 0 prep it for D3D
		d3DWindowRect.right -= d3DWindowRect.left;
		d3DWindowRect.left = d3DWindowRect.top;
		d3DWindowRect.top = d3DWindowRect.bottom;
		d3DWindowRect.bottom = d3DWindowRect.left;
		d3DWindowRect.left = 0;

		renderer.OnWindowResize(d3DWindowRect);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
