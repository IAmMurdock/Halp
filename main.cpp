#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include <dinput.h>

#include "camera.h"
#include "text2D.h"
#include "Model.h"
#include "SceneNode.h"

//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pBackBufferRTView = NULL;
Model*					g_pModel = NULL;

//camera
camera* gamecamera = NULL;
Text2D* g_2DText;
ID3D11ShaderResourceView* g_pTexture0;

//lighting
XMVECTOR g_directional_light_shines_from;
XMVECTOR g_directional_light_colour;
XMVECTOR g_ambient_light_colour;

//model
//Model* my_model;
//Model* model_2;
//Model* model3;
//Model* model4;
Model* arenaFloor;
Model* wallBack;
Model* wallFront;
Model* wallLeft;
Model* wallRight;


Model* cameraModel;

//input

IDirectInput8* g_direct_input;
IDirectInputDevice8* g_keyboard_device;
unsigned char g_keyboard_keys_state[256];

//scene nodes
 
SceneNode* g_root_node;
SceneNode* g_root_node2;
SceneNode* g_node1;
SceneNode* g_node2;
SceneNode* g_node3;
SceneNode* g_camera_node;
SceneNode* g_camera_cube;


ID3D11Buffer* g_pVertexBuffer;

ID3D11VertexShader* g_pVertexShader;

ID3D11PixelShader* g_pPixelShader;

ID3D11InputLayout* g_pInputLayout;

ID3D11Buffer* g_pConstantBuffer0;

float degrees = 0;

ID3D11DepthStencilView* g_pZBuffer;

ID3D11SamplerState* g_pSampler0; // Declared with other global variables at start of main()

struct CONSTANT_BUFFER0
{

	XMMATRIX WorldViewProjectionMatrix; //64 bytes
	XMVECTOR directional_light_vector;	// 16 bytes
	XMVECTOR directional_light_colour;	// 16 bytes
	XMVECTOR ambient_light_colour;		// 16 bytes

	//float RedAmmount;
	//float scale;

	XMFLOAT2 packing_bytes;

};

// Define vertex structure

struct POS_COL_TEX_NORM_VERTEX

{
	XMFLOAT3 Pos;

	XMFLOAT4 Col;

	XMFLOAT2 texture0;

	XMFLOAT3 normal;
};

// Rename for each tutorial
char		g_TutorialName[100] = "Delilah";


//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();

void ShutdownD3D();
void RenderFrame(void);
HRESULT InitialiseGraphics(void);
HRESULT DirectInputStart();

void ReadInputStates();
bool IsKeyPressed(unsigned char DI_Keycode);


//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}
	
	// Main message loop
	MSG msg = { 0 };

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	if (FAILED(InitialiseGraphics()))

	{

		DXTRACE_MSG("Failed to initialise graphics");

		return 0;

	}

	if (FAILED(DirectInputStart()))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// do something
			RenderFrame();
			/*my_model->CalculateModelCenterPoint();
			model_2->CalculateModelCenterPoint();*/
			//my_model->CalculateBoundingSphereRadius();
		}
	}

	ShutdownD3D();

	return (int)msg.wParam;
}

void ReadInputStates()
{
	HRESULT hr;
	hr = g_keyboard_device->GetDeviceState(sizeof(g_keyboard_keys_state),
		(LPVOID)&g_keyboard_keys_state);
	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			g_keyboard_device->Acquire();
		}
	}
}

bool IsKeyPressed(unsigned char DI_keycode)
{
	return g_keyboard_keys_state[DI_keycode] & 0x80;
}

HRESULT DirectInputStart()
{
	HRESULT hr;
	ZeroMemory(g_keyboard_keys_state, sizeof(g_keyboard_keys_state));

	hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_direct_input, NULL);
	if (FAILED(hr)) return hr;

	hr = g_direct_input->CreateDevice(GUID_SysKeyboard, &g_keyboard_device, NULL);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->Acquire();
	if (FAILED(hr)) return hr;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Dave Horne\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		g_root_node->MoveForward(10.0f);
		break;

	case WM_RBUTTONDOWN:
		g_node1->MoveForward(5.0f);
		break;


	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;
	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	//create a z buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D *pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr)) return hr;

	
	// Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();

	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);


	g_2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);


	return S_OK;

}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_keyboard_device)
	{
		g_keyboard_device->Unacquire();
		g_keyboard_device->Release();
	}

	if (g_direct_input) g_direct_input->Release();

	gamecamera = NULL;
	delete gamecamera;

	/*g_2DText = NULL;
	delete g_2DText;

	model_2 = NULL;
	delete model_2;

	my_model = NULL;
	delete my_model; */

	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pInputLayout) g_pInputLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();

	//delete texture
	if (g_pTexture0) g_pTexture0->Release();
	if (g_pSampler0) g_pSampler0->Release();

	if (g_pConstantBuffer0) g_pConstantBuffer0->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
}


HRESULT InitialiseGraphics()

{

	gamecamera = new camera(0.0, 2.0, -0.5, 0);

	//my_model = new Model(g_pD3DDevice, g_pImmediateContext);
	//my_model->LoadObjModel("assets/floor.obj");


	//model_2 = new Model(g_pD3DDevice, g_pImmediateContext);
	//model_2->LoadObjModel("assets/cube.obj");


	//model3 = new Model(g_pD3DDevice, g_pImmediateContext);
	//model3->LoadObjModel("assets/sphere.obj");

	//model4 = new Model(g_pD3DDevice, g_pImmediateContext);
	//model4->LoadObjModel("assets/floor.obj");


	cameraModel = new Model(g_pD3DDevice, g_pImmediateContext);
	cameraModel->LoadObjModel("assets/cube.obj");

	HRESULT hr = S_OK;

	g_root_node = new SceneNode();
	g_root_node2 = new SceneNode();
	g_node1 = new SceneNode();
	g_node2 = new SceneNode();
	g_node3 = new SceneNode();
	g_camera_node = new SceneNode();
	g_camera_cube = new SceneNode();

	g_camera_cube->setModel(cameraModel);
	g_camera_node->addChildNode(g_camera_cube);

	g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());

	//g_node1->setModel(my_model);
	//g_node2->setModel(model_2);

	//g_node3->setModel(model3);

	//g_root_node->addChildNode(g_node1);
	//g_node1->addChildNode(g_node2);

	//g_root_node2->addChildNode(g_node3);

	//g_root_node->setPosition(00, 000, 0);
	//g_node1->setPosition(10, 00, 00);
	//g_root_node2->setPosition(10, 000, 00);

	//TUTORIAL 14 EX13//
	/*g_node1->setPosition(g_root_node->getPositionX() + 5, g_root_node->getPositionY() + 5, g_root_node->getPositionZ() + 5);
	g_node2->setPosition(g_node1->getPositionX() + 5, g_node1->getPositionY() + 5, g_node1->getPositionZ() + 5);

	g_node3->setPosition(g_root_node->getPositionX() + 5, g_root_node->getPositionY() + 30, g_root_node->getPositionZ() + 5);*/
	
	//create camera

	//Create Arena Base
	arenaFloor = new Model(g_pD3DDevice, g_pImmediateContext);
	arenaFloor->LoadObjModel("assets/cube.obj");
	arenaFloor->setScale(100.0f, 0.02f, 100.0f);
	arenaFloor->setPosition(0.0f, 0.0f, 0.0f);
	arenaFloor->SetTexture();

	wallLeft = new Model(g_pD3DDevice, g_pImmediateContext);
	wallLeft->LoadObjModel("assets/cube.obj");
	wallLeft->setScale(0.20f, 40.0f, 100.0f);
	wallLeft->setPosition(-50.0f, 20.0f, 0.0f);

	wallRight = new Model(g_pD3DDevice, g_pImmediateContext);
	wallRight->LoadObjModel("assets/cube.obj");
	wallRight->setScale(0.20f, 40.0f, 100.0f);
	wallRight->setPosition(50.0f, 20.0f, 0.0f);

	wallFront = new Model(g_pD3DDevice, g_pImmediateContext);
	wallFront->LoadObjModel("assets/cube.obj");
	wallFront->setScale(100.0f, 40.0f, 0.2f);
	wallFront->setPosition(0.0f,20.0f, 150.0f);

	wallBack = new Model(g_pD3DDevice, g_pImmediateContext);
	wallBack->LoadObjModel("assets/cube.obj");
	wallBack->setScale(100.0f, 40.0f, 0.2f);
	wallBack->setPosition(0.0f, 20.0f, 50.0f);





	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	return S_OK;

}




void RenderFrame()
{
	ReadInputStates();

	if (IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);
	if (IsKeyPressed(DIK_LEFT))
	{
		gamecamera->side(0.11);
		g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			gamecamera->side(-.3);
			g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());
		}


	}
	if (IsKeyPressed(DIK_RIGHT))
	{
		gamecamera->side(-0.11);
		g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			gamecamera->side(.3);
			g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());
		}


	}
	if (IsKeyPressed(DIK_UP)) 
	{
		gamecamera->forward(0.11);
		g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			gamecamera->forward(-.3);
			g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());
		}


	}
	if (IsKeyPressed(DIK_DOWN)) 
	{
		gamecamera->forward(-0.11);
		g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			gamecamera->forward(.3);
			g_camera_node->setPosition(gamecamera->GetX(), gamecamera->GetY(), gamecamera->GetZ());
		}


	}

	if (IsKeyPressed(DIK_W)) arenaFloor->MoveForward(0.01);
	if (IsKeyPressed(DIK_S)) arenaFloor->MoveForward(-0.01);
	//if (IsKeyPressed(DIK_D)) g_root_node2->MoveRight(.001f, g_root_node2);
	//if (IsKeyPressed(DIK_A)) g_root_node2->MoveRight(-.001f, g_root_node2);

	if (IsKeyPressed(DIK_Q)) gamecamera->rotate(0.1);
	if (IsKeyPressed(DIK_E)) gamecamera->rotate(-0.1);



	//set lighting
	g_directional_light_shines_from = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	g_directional_light_colour = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
	g_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);


	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);

	g_pImmediateContext->ClearDepthStencilView
		(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	XMMATRIX projection, world;

	world = XMMatrixRotationY(XMConvertToRadians(degrees));
	world *= XMMatrixTranslation(1, 0, 25);

	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0, 1000.0);
	
	// RENDER HERE

	//my_model->LookAt_XZ(gamecamera->GetX(), gamecamera->GetY());
	//my_model->draw(&gamecamera->getviewmatrix(), &projection);

	//g_root_node->execute(&world, &gamecamera->getviewmatrix(), &projection );
	////g_root_node2->execute(&world, &gamecamera->getviewmatrix(), &projection);
	//model4->draw(&world, &gamecamera->getviewmatrix(), &projection);

	arenaFloor->draw(&gamecamera->getviewmatrix(), &projection);

	wallLeft->draw(&gamecamera->getviewmatrix(), &projection);
	wallRight->draw(&gamecamera->getviewmatrix(), &projection);
	wallFront->draw(&gamecamera->getviewmatrix(), &projection);
	wallBack->draw(&gamecamera->getviewmatrix(), &projection);
	
	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}
