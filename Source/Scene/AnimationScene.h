#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Model.h"

class AnimationScene : public Scene
{
public:
	AnimationScene();
	~AnimationScene() override = default;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render(float elapsedTime) override;

	// GUI描画処理
	void DrawGUI() override;

private:
	// アニメーション再生
	void PlayAnimation(int index, bool loop);
	void PlayAnimation(const char* name, bool loop);

	// アニメーション更新処理
	void UpdateAnimation(float elapsedTime);

	// トランスフォーム更新処理
	void UpdateTransform(float elapsedTime);

	// 移動入力処理
	bool InputMove();

	// ジャンプ入力処理
	bool InputJump();

private:
	Camera								camera;
	FreeCameraController				cameraController;

	std::unique_ptr<Model>				model;
	DirectX::XMFLOAT4X4					transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT3					position = { 0, 0, 0 };
	DirectX::XMFLOAT3					angle = { 0, 0, 0 };
	DirectX::XMFLOAT3					scale = { 0.01f, 0.01f, 0.01f };

	DirectX::XMFLOAT3					velocity = { 0, 0, 0 };
	float								gravity = 10.0f;
	float								acceleration = 50.0f;
	float								deceleration = 20.0f;
	float								moveSpeed = 5.0f;
	float								turnSpeed = DirectX::XMConvertToRadians(720);
	float								jumpSpeed = 5.0f;
	float								airControl = 0.3f;
	float								moveVecX = 0.0f;
	float								moveVecZ = 0.0f;
	bool								onGround = false;


	int									animationIndex = -1;
	float								animationSeconds = 0.0f;
	bool								animationLoop = false;
	bool								animationPlaying = false;
	float animationBlendTime = 0.0f;
	float								animationBlendSecondsLength = 0.2f;
	float blendRate = 1.0f;

	enum class State
	{
		Idle,
		Run,
		Jump,
	};
	State								state = State::Idle;
};
