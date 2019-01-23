#include "main.h"
#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <wrl.h>
#include <ShlObj.h>
#include <system_error>
#include <string>
#include <filesystem>
#include <wincodec.h>
#include <cstdio>
#include <MinHook.h>
#include <cassert>
#include <chrono>
#include "export.h"
#include "script.h"
#include <d3d11shader.h>
#include <queue>
#include <d3dcompiler.h>
#include <vector>
#include <Eigen/Core>
#include <atlimage.h>
#include <fstream>
#include <ScreenGrab.h>
#include "script.h"
#include "keyboard.h"
#include "infoIO.h"
using Microsoft::WRL::ComPtr;
using namespace std::experimental::filesystem;
using namespace std::string_literals;
using std::chrono::milliseconds;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::vector;
using Eigen::Matrix4f;
using Eigen::Vector3f;
using Eigen::Vector4f;
typedef void(*draw_indexed_hook_t)(ID3D11DeviceContext*, UINT, UINT, INT);
void presentCallback(void* chain);

// void scriptMain();

//void draw_indexed_hook(ID3D11DeviceContext3* self, UINT IndexStart, UINT StartIndexLocation, INT BaseVertexLocation);
static time_point<system_clock> last_capture_color;
static time_point<system_clock> last_capture_depth;
static const char* logFilePath = "GTANativePlugin.log";
//--------
//offsets
//--------
const size_t drawIndexedOffset = 12;
const size_t clearDepthStencilViewOffset = 53;
//-------------------------
//interesting D3D resources
//-------------------------
static ComPtr<ID3D11DepthStencilView> lastDsv;
static ComPtr<ID3D11RenderTargetView> lastRtv;
static ComPtr<ID3D11Buffer> lastConstants;

static bool saveNextFrame = false;
static bool hooked = false;

//-------------------------
//global control variables
//-------------------------

static bool draw_indexed_count = false;

enum catchState
{
	catchStop,
	catchStart,
	catchScreen
};

static catchState cmdToCatch = catchStop;
static WCHAR imgPath[fileLength] = L"screen.bmp";
static char rawpath[fileLength] = "stencil.raw";
static bool onlyScreen = false, forceSave = false;

inline void makeCmdStart()
{
	cmdToCatch = catchStart;
}
inline void makeCmdStop()
{
	cmdToCatch = catchStop;
}
inline void cmdCatchScreen()
{
	cmdToCatch = catchScreen;
}

void catchCurveAndScreen(WCHAR *_imgPath, char *_rawPath, bool _forceSave, bool _onlyScreen)
{
	wcscpy(imgPath, _imgPath);
	onlyScreen = _onlyScreen;
	forceSave = _forceSave;
	if(!onlyScreen) strcpy(rawpath, _rawPath);
	makeCmdStart();
}

static void (_stdcall ID3D11DeviceContext::* origDrawInstanced)(UINT, UINT, INT) = nullptr;
int __stdcall DllMain(HMODULE hinstance, DWORD reason, LPVOID lpReserved)
{
	MH_STATUS res;
	auto f = fopen(logFilePath, "a");
	switch(reason)
	{
	case DLL_PROCESS_ATTACH:
		res = MH_Initialize();
		if (res != MH_OK) fprintf(f, "Could not init Minihook\n");
		presentCallbackRegister(presentCallback);
		keyboardHandlerRegister(OnKeyboardMessage);
		scriptRegister(hinstance, scriptMain);
		break;
	case DLL_PROCESS_DETACH:
		res = MH_Uninitialize();
		if (res != MH_OK) fprintf(f, "Could not deinit MiniHook\n");
		presentCallbackUnregister(presentCallback);
		keyboardHandlerUnregister(OnKeyboardMessage);
		//scriptUnregister(hinstance);

		break;
	}
	fclose(f);
	return TRUE;
}

template<int offset, typename T>
static void* orig;
template<int offset, typename T>
static void* targets;
template<int offset, typename T>
void hook_function(T* inst, void* hook, bool unhook = false)
{
	//__debugbreak();
	void** vtbl = *reinterpret_cast<void***>(inst);
	FILE* f = fopen("GTANativePlugin.log", "a");
	//fprintf(f, "Hooking %p at offset %d\n", inst, offset);
	MH_STATUS res = MH_OK;
	DWORD oldProt = 0;
	vtbl += offset;
	//VirtualProtect(vtbl, 8, PAGE_READWRITE, &oldProt);
	if (unhook)
	{
		res = MH_DisableHook(vtbl);
		if(res != MH_OK) fprintf(f, "error %d disabling hook at offset %d\n", res,  offset);
		orig<offset, T> = nullptr;
	}
	else {
		if(targets<offset, T> != nullptr && targets<offset, T> != *vtbl)
		{
			fprintf(f, "detected target change, someone else is screwing with our functions\n");
			res = MH_DisableHook(targets<offset, T>);
			if (res != MH_OK) fprintf(f, "errof %d disabling hook at offset %d\n", res, offset);
			res = MH_RemoveHook(targets<offset, T>);
			if (res != MH_OK) fprintf(f, "error %d removing hook at offset %d\n", res, offset);
			targets<offset, T> = nullptr;
			orig<offset, T> = nullptr;
		}
		if (orig<offset, T> == nullptr && targets<offset, T> != *vtbl) {
			std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
				std::chrono::system_clock::now().time_since_epoch()
				);
			//fprintf(f, "[%I64d] :  create hook\n", ms.count());
			res = MH_CreateHook(*vtbl, hook, &(orig<offset, T>));
			if(res != MH_OK) fprintf(f, "error %d creating hook at offset %d\n",res, offset);
			
		}
		if (targets<offset, T> != *vtbl) {
			res = MH_EnableHook(*vtbl);
			if (res != MH_OK) fprintf(f, "error %d enabling hook at offset %d\n", res, offset);
			targets<offset, T> = *vtbl;
		}
		//*vtbl = reinterpret_cast<long long>(hook);
	}
	//VirtualProtect(vtbl, 8, oldProt, nullptr);
	//fprintf(f, "clear_hook: %p\n", hook);
	//fprintf(f, "clearFn: %p\n", (void*)(*(*reinterpret_cast<long long**>(inst) + 50)));
	fclose(f);
}

template<int offset, typename T>
void unhook_function(T* inst)
{
	hook_function<offset>(inst, nullptr, true);
}
void draw_hook_impl()
{
	FILE* f = fopen("DrawLog.log", "a");
	fprintf(f, "Draw Call\n");
	fclose(f);
}
void draw_indexed_hook(ID3D11DeviceContext* self, UINT indexCount, UINT startLoc, UINT baseLoc) {
	auto origMethod = reinterpret_cast<decltype(draw_indexed_hook)*>(orig<drawIndexedOffset, ID3D11DeviceContext>);
	HRESULT hr;
	ComPtr<ID3D11VertexShader> vs;
	self->VSGetShader(&vs, nullptr, nullptr);
	ComPtr<ID3D11Buffer> buf;
	ComPtr<ID3D11Device> dev;
	self->GetDevice(&dev);
	self->VSGetConstantBuffers(1, 1, &buf);
	if (buf != nullptr && draw_indexed_count == 1000) {
		lastConstants = buf;
		ExtractConstantBuffer(dev.Get(), self, buf.Get());
	}
	
	draw_indexed_count += 1;
	origMethod(self, indexCount, startLoc, baseLoc);
}
void clear_render_target_view_hook(ID3D11DeviceContext* self, ID3D11RenderTargetView* rtv, float color[4])
{
	auto origMethod = reinterpret_cast<void (*)(ID3D11DeviceContext*, ID3D11RenderTargetView*, float[4])>(orig<50, ID3D11DeviceContext>);
	
	ComPtr<ID3D11RenderTargetView> curRTV;
	self->OMGetRenderTargets(1, &curRTV, nullptr);
	if (curRTV != nullptr)
	{
		D3D11_TEXTURE2D_DESC desc;
		ComPtr<ID3D11Resource> res;
		ComPtr<ID3D11Texture2D> tex;
		curRTV->GetResource(&res);
		HRESULT hr = S_OK;
		hr = res.As(&tex);
		if (hr != S_OK) return;
		tex->GetDesc(&desc);
		if (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM && desc.Width > 600 && desc.Height > 600) {
			lastRtv = curRTV;
		}
	}
	origMethod(self, rtv, color);
}

auto screenShot = [](FILE* f) {
	int screenCapResult = export_get_screen_buffer(imgPath);
	std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::system_clock::now().time_since_epoch()
		);
	if (screenCapResult != 1) {
		fprintf(f, "[%I64d] : export screen %ls failed.\n", ms, imgPath);
	}
	else {
		fprintf(f, "[%I64d] : export screen %ls success.\n", ms, imgPath);
	}
	wcscpy(imgPath, L"screen.bmp");
};

void clear_depth_stencil_view_hook(ID3D11DeviceContext* self, ID3D11DepthStencilView* dsv, UINT8 flags, float depth, UINT8 stencil)
{
	auto origMethod = reinterpret_cast<decltype(&clear_depth_stencil_view_hook)>(orig<53, ID3D11DeviceContext>);
	ComPtr<ID3D11DepthStencilView> curDSV;
	self->OMGetRenderTargets(1, nullptr, &curDSV);
	ComPtr<ID3D11Device> dev;
	self->GetDevice(&dev);
	if (curDSV != nullptr) {
		D3D11_TEXTURE2D_DESC desc;
		ComPtr<ID3D11Resource> res;
		ComPtr<ID3D11Texture2D> tex;
		curDSV->GetResource(&res);
		HRESULT hr = S_OK;
		hr = res.As(&tex);
		if (hr != S_OK) return;
		tex->GetDesc(&desc);
		
		if (lastDsv == nullptr && desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS && desc.Width > 600 && desc.Height > 600) {
			lastDsv = curDSV;
			std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
				std::chrono::system_clock::now().time_since_epoch()
				);
			FILE* f = fopen(logFilePath, "a");
			//go = true;
			//fprintf(f, "[%I64d] : trans stencil info over, cmdToCatch = %d.\n", ms.count(), cmdToCatch);
			
			ExtractDepthBuffer(dev.Get(), self, res.Get());
			last_capture_depth = system_clock::now();

			if (cmdToCatch == catchStart) {
				void *buf;
				int size = export_get_stencil_buffer(&buf);
				fprintf(f, "[%I64d] : onlyscreen = %s.\n", ms, onlyScreen ? "yes" : "no");
				if (!onlyScreen) {
					screenShot(f);
					auto raw = fopen(rawpath, "w");
					fwrite(buf, 1, size, raw);
					fclose(raw);
					fprintf(f, "[%I64d] : write stencil %s into file.\n", ms, rawpath);
					strcpy(rawpath, "stencil.raw");
				}
				else {
					onlyScreen = false;
					bool writeBmp = true;
					if (!forceSave) {
						for (int i = 0; i < size; i++) {
							if (((unsigned char*)buf)[i] == unsigned char(1)) {
								writeBmp = false;
								break;
							}
						}
					}
					if(writeBmp) screenShot(f);
				}
				makeCmdStop();
			}
			fclose(f);
		}
	}
	origMethod(self, dsv, flags, depth, stencil);
	
	
}


void presentCallback(void* chain)
{	
	FILE* f = fopen(logFilePath, "a");
	std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::system_clock::now().time_since_epoch()
		);

	draw_indexed_count = 0;
	HRESULT hr2 = S_OK, hr1 = S_OK;
	ComPtr<ID3D11Device> dev;
	ComPtr<ID3D11DeviceContext> ctx;
	ComPtr<ID3D11Texture2D> backBuffer;

	auto swapChain = static_cast<IDXGISwapChain*>(chain);
	hr2 = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
	if (hr2 != S_OK) throw std::system_error(hr2, std::system_category());

	swapChain = static_cast<IDXGISwapChain*>(chain);
	hr1 = swapChain->GetDevice(__uuidof(ID3D11Device), &dev);
	if (hr1 != S_OK) throw std::system_error(hr1, std::system_category());
	dev->GetImmediateContext(&ctx);

	ExtractScreenBuffer(ctx.Get(), backBuffer.Get(), hr2);
	ComPtr<ID3D11Multithread> multithread;
	hr2 = ctx.As(&multithread);
	if (hr2 != S_OK) throw std::system_error(hr2, std::system_category());
	multithread->SetMultithreadProtected(true);
	hook_function<drawIndexedOffset>(ctx.Get(), &draw_indexed_hook);
	
	hook_function<53>(ctx.Get(), &clear_depth_stencil_view_hook);
	if (f == nullptr) throw std::system_error(errno, std::system_category());
	
	ComPtr<ID3D11Resource> depthres;
	ComPtr<ID3D11Resource> colorres;
	ctx->OMGetRenderTargets(1, &lastRtv, nullptr);
	last_capture_color = system_clock::now();
	lastRtv->GetResource(&colorres);
	ExtractColorBuffer(dev.Get(), ctx.Get(), colorres.Get());
	//lastDsv.Reset();

	lastDsv = nullptr;
	lastRtv = nullptr;

	ms = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::system_clock::now().time_since_epoch()
		);

	fclose(f);
}