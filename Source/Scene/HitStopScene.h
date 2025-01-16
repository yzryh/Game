#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Model.h"

class HitStopScene : public Scene
{
public:
	HitStopScene();
	~HitStopScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:

	float Easing(float x)
	{
		return 1 - powf(1 - x, 4);
	}

private:
	struct Object
	{
		DirectX::XMFLOAT3		position = { 0, 0, 0 };
		DirectX::XMFLOAT3		angle = { 0, 0, 0 };
		DirectX::XMFLOAT3		scale = { 1, 1, 1 };
		DirectX::XMFLOAT4X4		transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		std::unique_ptr<Model>	model;
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Object								character;
	Object								weapon;
	float								characterHitOffset = 1.0f;
	float								characterHitRadius = 0.5f;
	float								weaponHitOffset = 1.5f;
	float								weaponHitRadius = 0.5f;
	float								weaponMotionCurrentSeconds = 0.0f;
	float								weaponMotionSecondsLength = 1.0f;
	bool								hitStop = false;

	// ANS�@-�@�q�b�g�X�g�b�v�ƃJ�����V�F�C�N�ɕK�v�ȕϐ�
	float								hitStopLastSeconds = 0.0f;
	float								hitStopSecondsLength = 0.3f;
	float                               hitbb = 0.3f;
	float								cameraShakeRange = 0.03f;
};
