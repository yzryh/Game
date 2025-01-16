#pragma once

#include <DirectXMath.h>

class TransformUtils
{
public:
	// �s�񂩂烈�[�A�s�b�`�A���[�����s����v�Z����B
	static bool MatrixToRollPitchYaw(const DirectX::XMFLOAT4X4& m, float& pitch, float& yaw, float& roll);

	// �N�H�[�^�j�I�����烈�[�A�s�b�`�A���[�����s����v�Z����B
	static bool QuaternionToRollPitchYaw(const DirectX::XMFLOAT4& q, float& pitch, float& yaw, float& roll);
};
