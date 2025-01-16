#pragma once
#include "Camera.h"

class FreeCameraController
{
public:
	// �J��������R���g���[���[�փp�����[�^�𓯊�����
	void SyncCameraToController(const Camera& camera);

	// �R���g���[���[����J�����փp�����[�^�𓯊�����
	void SyncControllerToCamera(Camera& camera);

	// �X�V����
	void Update();

private:
	DirectX::XMFLOAT3		eye;
	DirectX::XMFLOAT3		focus;
	DirectX::XMFLOAT3		up;
	DirectX::XMFLOAT3		right;
	float					distance;

	float					angleX;
	float					angleY;
};
