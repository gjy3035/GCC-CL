#ifndef GTA_VISION_NATIVE_EXPORT_H
#define GTA_VISION_NATIVE_EXPORT_H
#include <d3d11.h>
#include <atlimage.h>
#include <Eigen/Core>
#include <string>

void ExtractDepthBuffer(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Resource* tex);
void ExtractColorBuffer(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Resource* tex);
void ExtractConstantBuffer(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Buffer* buf);
void ExtractScreenBuffer(ID3D11DeviceContext* ctx, ID3D11Texture2D* back, HRESULT hr);
void CopyIfRequested();
void writeLog(std::string);

struct rage_matrices {
	Eigen::Matrix4f M;
	Eigen::Matrix4f MV;
	Eigen::Matrix4f MVP;
	Eigen::Matrix4f Vinv;
}; 

extern "C" {
	__declspec(dllexport) int export_get_depth_buffer(void** buf);
	__declspec(dllexport) int export_get_color_buffer(void** buf);
	__declspec(dllexport) int export_get_stencil_buffer(void** buf);
	__declspec(dllexport) int export_get_constant_buffer(rage_matrices* buf);
	__declspec(dllexport) int export_get_screen_buffer(WCHAR *pictureName);
}
#endif