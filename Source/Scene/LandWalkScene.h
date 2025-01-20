#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Model.h"
#include "Sprite.h"

class LandWalkScene : public Scene
{
public:
	LandWalkScene();
	~LandWalkScene() override = default;

	// çXêVèàóù
	void Update(float elapsedTime) override;

	// ï`âÊèàóù
	void Render(float elapsedTime) override;

	// GUIï`âÊèàóù
	void DrawGUI() override;

private:
	struct Object
	{
		bool					onGround = false;
		int						hp = 2000;
		DirectX::XMFLOAT3		velocity = { 0, 0, 0 };
		DirectX::XMFLOAT3		position = { 0, 0, 0 };
		DirectX::XMFLOAT3		angle = { 0, 0, 0 };
		DirectX::XMFLOAT3		scale = { 1, 1, 1 };
		DirectX::XMFLOAT4X4		transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		std::unique_ptr<Model>	model;

		void UpdateTransform()
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
			DirectX::XMMATRIX WorldTransform = S * R * T;
			DirectX::XMStoreFloat4x4(&transform, WorldTransform);
		}
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Object								player;
	Object								player2;
	Object								stage;
	std::unique_ptr<Sprite>				sprite;
	float								gravity = 10.0f;
	float								acceleration = 30.0f;
	float								deceleration = 20.0f;
	float								moveSpeed = 5.0f;
	float								turnSpeed = DirectX::XMConvertToRadians(720);
	float								jumpSpeed = 5.0f;
	float								downhillOffset = 43.0f;
};
