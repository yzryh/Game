#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Timer.h"
#include "Model.h"
#include "Sprite.h"

class ResourceManagementScene : public Scene
{
public:
	ResourceManagementScene();
	~ResourceManagementScene() override = default;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render(float elapsedTime) override;

	// GUI描画処理
	void DrawGUI() override;

private:
	struct Object
	{
		DirectX::XMFLOAT3		position = { 0, 0, 0 };
		DirectX::XMFLOAT3		angle = { 0, 0, 0 };
		DirectX::XMFLOAT3		scale = { 1, 1, 1 };
		DirectX::XMFLOAT4X4		transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		std::unique_ptr<Model>	model;
	};

	Timer								timer;
	Camera								camera;
	FreeCameraController				cameraController;
	std::unique_ptr<Sprite>				sprite;
	Object								stage;
	std::vector<Object>					objs;
	float								loadTime = 0;
};
