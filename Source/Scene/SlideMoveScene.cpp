#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/SlideMoveScene.h"

// �R���X�g���N�^
SlideMoveScene::SlideMoveScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// �J�����ݒ�
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),	// ��p
		screenWidth / screenHeight,			// ��ʃA�X�y�N�g��
		0.1f,								// �j�A�N���b�v
		1000.0f								// �t�@�[�N���b�v
	);
	camera.SetLookAt(
		{ 13, 10, 27 },		// ���_
		{ 13, 1, 16 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// ���f��
	stage.model = std::make_unique<Model>("Data/Model/Greybox/Greybox.mdl");

	player.position = { 13, 0, 16 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// �X�V����
void SlideMoveScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �v���C���[�ړ�����
	{
		// ���͏���
		float axisX = 0.0f;
		float axisY = 0.0f;
		if (GetAsyncKeyState('W') & 0x8000) axisY += 1.0f;
		if (GetAsyncKeyState('S') & 0x8000) axisY -= 1.0f;
		if (GetAsyncKeyState('D') & 0x8000) axisX += 1.0f;
		if (GetAsyncKeyState('A') & 0x8000) axisX -= 1.0f;

		// �J�����̕���
		const DirectX::XMFLOAT3& cameraFront = camera.GetFront();
		const DirectX::XMFLOAT3& camemraRight = camera.GetRight();
		float cameraFrontLengthXZ = sqrtf(cameraFront.x * cameraFront.x + cameraFront.z * cameraFront.z);
		float cameraRightLengthXZ = sqrtf(camemraRight.x * camemraRight.x + camemraRight.z * camemraRight.z);
		float cameraFrontX = cameraFront.x / cameraFrontLengthXZ;
		float cameraFrontZ = cameraFront.z / cameraFrontLengthXZ;
		float cameraRightX = camemraRight.x / cameraRightLengthXZ;
		float cameraRightZ = camemraRight.z / cameraRightLengthXZ;

		// �ړ��x�N�g��
		const float speed = 5.0f * elapsedTime;
		float vecX = cameraFrontX * axisY + cameraRightX * axisX;
		float vecZ = cameraFrontZ * axisY + cameraRightZ * axisX;
		float moveX = vecX * speed;
		float moveZ = vecZ * speed;
		float moveLength = sqrtf(moveX * moveX + moveZ * moveZ);

		// �ǂ���ړ�����
		if (moveLength > 0)
		{
			// ���C�̎n�_�ƏI�_�����߂�
			const DirectX::XMFLOAT3 s =
			{ player.position.x,
			  player.position.y + 0.5f,
			  player.position.z };
			const DirectX::XMFLOAT3 e = {
				player.position.x+moveX,
				player.position.y +0.5f,
				player.position.z+moveZ};
			// ���C�L���X�g���s���A��_�����߂�
			DirectX::XMFLOAT3 p, n;
			if (Collision::RayCast(s, e, stage.transform, stage.model.get(), p, n))
			{
				//��_����I�_�ւ̃x�N�g�������߂�
				DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&p);
				DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&e);
				DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);

				//�O�p�֐��ŏI�_����ǂ܂ł̒��������߂�
				DirectX::XMVECTOR N = DirectX::XMLoadFloat3(&n);
				DirectX::XMVECTOR A = DirectX::XMVector3Dot(DirectX::XMVectorNegate(PE), N);
				//�ǂ܂ł̒������������������Ȃ�悤�ɕ␳
				float a = DirectX::XMVectorGetX(A) + 0.001f;

				//�ǂ���x�N�g�������߂�
				DirectX::XMVECTOR R = DirectX::XMVectorAdd(PE, DirectX::XMVectorScale(N, a));

				//�ǂ����̈ʒu�����߂�
				DirectX::XMVECTOR Q = DirectX::XMVectorAdd(P, R);
				DirectX::XMFLOAT3 q;
				DirectX::XMStoreFloat3(&q, Q);
				//�Ǎۂŕǂ����̈ʒu���߂荞��łȂ������C�L���X�g�Ń`�F�b�N����
				if (Collision::RayCast(s, q, stage.transform, stage.model.get(), p, n))
				{
					P = DirectX::XMLoadFloat3(&p);
					DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&s);
					DirectX::XMVECTOR PS = DirectX::XMVectorSubtract(S, P);
					DirectX::XMVECTOR V = DirectX::XMVector3Normalize(PS);
					P = DirectX::XMVectorAdd(P, DirectX::XMVectorScale(V, 0.001f));
					DirectX::XMStoreFloat3(&p, P);
					player.position.x = p.x;
					player.position.z = p.z;

				}
				else
				{
					player.position.x = q.x;
					player.position.z = q.z;
				}
			}
		else
		{
			player.position.x += moveX;
			player.position.z += moveZ;
		}
			// �i�s�����������悤�ɂ���
			float vx = moveX / moveLength;
			float vz = moveZ / moveLength;
			player.angle.y = atan2f(vx, vz);
		}

		if (moveLength > 0)
			// �ڒn����
		{
			// ���C�̎n�_�ƏI�_�����߂�
			const DirectX::XMFLOAT3 s = { player.position.x, player.position.y + 0.5f, player.position.z };
			const DirectX::XMFLOAT3 e = { player.position.x, player.position.y - 3.0f, player.position.z };

			// ���C�L���X�g���s���A��_�����߂�
			DirectX::XMFLOAT3 p, n;
			if (Collision::RayCast(s, e, stage.transform, stage.model.get(), p, n))
			{
				// ��_��Y���W���v���C���[�Ɉʒu�ɐݒ肷��
				player.position.y = p.y;
			}
		}
		// �v���C���[�s��X�V����
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(player.angle.x, player.angle.y, player.angle.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
			DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
		}
	}
}

// �`�揈��
void SlideMoveScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// ���f���`��
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, stage.transform, stage.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);
}

// GUI�`�揈��
void SlideMoveScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"�ǂ���ړ�", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"�ړ�����FWASD");

		ImGui::DragFloat3("Position", &player.position.x, 0.1f);
	}
	ImGui::End();
}
