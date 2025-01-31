#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Model.h"
#include "Sprite.h"

class UIAnimScene : public Scene
{
public:
	UIAnimScene();
	~UIAnimScene() override = default;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render(float elapsedTime) override;

	// GUI描画処理
	void DrawGUI() override;

private:
	// スプライト描画
	void DrawSprite(
		ID3D11DeviceContext* dc,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float screenWidth,
		float screenHeight) const;

	// 水平塗りつぶし描画
	void DrawFillHorizontal(
		ID3D11DeviceContext* dc,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float amount,
		float screenWidth,
		float screenHeight) const;

	// 垂直塗りつぶし描画
	void DrawFillVertical(
		ID3D11DeviceContext* dc,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float amount,
		float screenWidth,
		float screenHeight) const;

	// イージング描画
	void DrawEasing(
		ID3D11DeviceContext* dc,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float amount,
		float screenWidth,
		float screenHeight) const;

	// 放射塗りつぶし描画
	void DrawFillRadial(
		ID3D11DeviceContext* dc,
		float dx, float dy, float dw, float dh,
		float sx, float sy, float sw, float sh,
		float amount,
		float screenWidth,
		float screenHeight) const;

private:
	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexBuffer;

	D3D11_TEXTURE2D_DESC	textureDesc;

	float easingAmount = 1.0f;
	float horizontalFillAmount = 1.0f;
	float verticalFillAmount = 1.0f;
	float radialFillAmount = 1.0f;
};
