#pragma once

#include <DirectXMath.h>

struct DirectionalLight
{
	DirectX::XMFLOAT3	direction = { 0, -1, 0 };
	DirectX::XMFLOAT3	color = { 1, 1, 1 };
};

class LightManager
{
public:
	// �f�B���N�V���i�����C�g�ݒ�
	void SetDirectionalLight(DirectionalLight& light) { directionalLight = light; }

	// �f�B���N�V���i�����C�g�擾
	const DirectionalLight& GetDirectionalLight() const { return directionalLight; }

private:
	DirectionalLight	directionalLight;
};
