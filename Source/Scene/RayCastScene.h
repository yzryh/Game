#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "Model.h"
#include "FreeCameraController.h"

class RayCastScene : public Scene
{
public:
	RayCastScene();
	~RayCastScene() override = default;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render(float elapsedTime) override;

	// GUI描画処理
	void DrawGUI() override;

private:
	// レイキャスト
	static bool RayCast(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		const DirectX::XMFLOAT3& vertexA,
		const DirectX::XMFLOAT3& vertexB,
		const DirectX::XMFLOAT3& vertexC,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal);

private:
	Camera								camera;
	FreeCameraController				cameraController;
	std::unique_ptr<Model>				model;
	DirectX::XMFLOAT4X4					worldTransform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};
