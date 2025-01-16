#include <imgui.h>
#include "FreeCameraController.h"

// �J��������R���g���[���[�փp�����[�^�𓯊�����
void FreeCameraController::SyncCameraToController(const Camera& camera)
{
	eye = camera.GetEye();
	focus = camera.GetFocus();
	up = camera.GetUp();
	right = camera.GetRight();

	// ���_���璍���_�܂ł̋������Z�o
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Focus, Eye);
	DirectX::XMVECTOR Distance = DirectX::XMVector3Length(Vec);
	DirectX::XMStoreFloat(&distance, Distance);

	// ��]�p�x���Z�o
	const DirectX::XMFLOAT3& front = camera.GetFront();
	angleX = ::asinf(-front.y);
	if (up.y < 0)
	{
		if (front.y > 0)
		{
			angleX = -DirectX::XM_PI - angleX;
		}
		else
		{
			angleX = DirectX::XM_PI - angleX;
		}
		angleY = ::atan2f(front.x, front.z);
	}
	else
	{
		angleY = ::atan2f(-front.x, -front.z);
	}

}

// �R���g���[���[����J�����փp�����[�^�𓯊�����
void FreeCameraController::SyncControllerToCamera(Camera& camera)
{
	camera.SetLookAt(eye, focus, up);
}

// �X�V����
void FreeCameraController::Update()
{
	// �f�o�b�O�E�C���h�E���쒆�͏������Ȃ�
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	{
		return;
	}

	// IMGUI�̃}�E�X���͒l���g���ăJ�������삷��
	ImGuiIO io = ImGui::GetIO();

	// �}�E�X�J�[�\���̈ړ��ʂ����߂�
	float moveX = io.MouseDelta.x * 0.02f;
	float moveY = io.MouseDelta.y * 0.02f;

	// �}�E�X���{�^��������
	if (io.MouseDown[ImGuiMouseButton_Right])
	{
		// Y����]
		angleY += moveX * 0.5f;
		if (angleY > DirectX::XM_PI)
		{
			angleY -= DirectX::XM_2PI;
		}
		else if (angleY < -DirectX::XM_PI)
		{
			angleY += DirectX::XM_2PI;
		}
		// X����]
		angleX += moveY * 0.5f;
		if (angleX > DirectX::XM_PI)
		{
			angleX -= DirectX::XM_2PI;
		}
		else if (angleX < -DirectX::XM_PI)
		{
			angleX += DirectX::XM_2PI;
		}
	}
	// �}�E�X���{�^��������
	else if (io.MouseDown[ImGuiMouseButton_Middle])
	{
		// ���s�ړ�
		float s = distance * 0.035f;
		float x = moveX * s;
		float y = moveY * s;

		focus.x -= right.x * x;
		focus.y -= right.y * x;
		focus.z -= right.z * x;

		focus.x += up.x * y;
		focus.y += up.y * y;
		focus.z += up.z * y;
	}
	// �}�E�X�E�{�^��������
	else if (io.MouseDown[ImGuiMouseButton_Left] && io.MouseDown[ImGuiMouseButton_Right])
	{
		// �Y�[��
		distance += (-moveY - moveX) * distance * 0.1f;
	}
	// �}�E�X�z�C�[��
	else if (io.MouseWheel != 0)
	{
		// �Y�[��
		distance -= io.MouseWheel * distance * 0.1f;
	}

	float sx = ::sinf(angleX);
	float cx = ::cosf(angleX);
	float sy = ::sinf(angleY);
	float cy = ::cosf(angleY);

	// �J�����̕������Z�o
	DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	DirectX::XMVECTOR Right = DirectX::XMVectorSet(cy, 0, -sy, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVector3Cross(Right, Front);
	// �J�����̎��_�������_���Z�o
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, 0.0f);
	DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, DirectX::XMVectorMultiply(Front, Distance));
	// �r���[�s�񂩂烏�[���h�s����Z�o
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMMATRIX World = DirectX::XMMatrixTranspose(View);
	// ���[���h�s�񂩂�������Z�o
	Right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), World);
	Up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), World);
	// ���ʂ��i�[
	DirectX::XMStoreFloat3(&eye, Eye);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&right, Right);
}
