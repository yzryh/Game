#pragma once

#include <Windows.h>
#include <d3d11.h>

class ImGuiRenderer
{
public:
	// ������
	static void Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* dc);
	
	// �I����
	static void Finalize();

	// �t���[���J�n����
	static void NewFrame();

	// �`����s
	static void Render(ID3D11DeviceContext* context);

	// WIN32���b�Z�[�W�n���h���[
	static LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
