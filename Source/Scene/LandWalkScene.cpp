#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/LandWalkScene.h"

// �R���X�g���N�^
LandWalkScene::LandWalkScene()
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

	player.position = { 13, 5, 16 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// �X�V����
void LandWalkScene::Update(float elapsedTime)
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
		if (GetAsyncKeyState(VK_SPACE) & 0x01) player.velocity.y = jumpSpeed;

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
		float vecX = cameraFrontX * axisY + cameraRightX * axisX;
		float vecZ = cameraFrontZ * axisY + cameraRightZ * axisX;
		float vecLength = sqrtf(vecX * vecX + vecZ * vecZ);

		// �������ړ�����
		if (vecLength > 0)
		{
			// �P�ʃx�N�g����
			vecX /= vecLength;
			vecZ /= vecLength;

			// ��������
			float acceleration = this->acceleration * elapsedTime;
			player.velocity.x += vecX * acceleration;
			player.velocity.z += vecZ * acceleration;

			// �ő呬�x����
			float velocityLength = sqrtf(player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z);
			if (velocityLength > moveSpeed)
			{
				player.velocity.x = (player.velocity.x / velocityLength) * moveSpeed;
				player.velocity.z = (player.velocity.z / velocityLength) * moveSpeed;
			}

			// �i�s�����������悤�ɂ���
			{
				// �v���C���[�̌����Ă������
				float frontX = sinf(player.angle.y);
				float frontZ = cosf(player.angle.y);

				// ��]�ʒ���
				float dot = frontX * vecX + frontZ * vecZ;
				float rot = (std::min)(1.0f - dot, turnSpeed * elapsedTime);

				// ���E��������ĉ�]����
				float cross = frontX * vecZ - frontZ * vecX;
				if (cross < 0.0f)
				{
					player.angle.y += rot;
				}
				else
				{
					player.angle.y -= rot;
				}
			}
		}
		else
		{
			// ��������
			float deceleration = this->deceleration * elapsedTime;
			float velocityLength = sqrtf(player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z);
			if (velocityLength > deceleration)
			{
				player.velocity.x -= (player.velocity.x / velocityLength) * deceleration;
				player.velocity.z -= (player.velocity.z / velocityLength) * deceleration;
			}
			else
			{
				player.velocity.x = 0.0f;
				player.velocity.z = 0.0f;
			}
		}

		// �d�͏���
		player.velocity.y -= gravity * elapsedTime;

		// �ړ���
		float moveX = player.velocity.x * elapsedTime;
		float moveY = player.velocity.y * elapsedTime;
		float moveZ = player.velocity.z * elapsedTime;
		
		// �ڒn����
		{
			/*bool onGround = player.onGround;
			player.onGround = false;
			if (player.velocity.y < 0.0f)
			{
				DirectX::XMFLOAT3 start, end;
				start = end = player.position;
				start.y = 0.5f;
				end.y += moveY;

				float downhillOffset = this->downhillOffset * elapsedTime;
				if (onGround)
				{
					end.y -= downhillOffset;
				}
				DirectX::XMFLOAT3 hitPosition, hitNormal;
				if(Collision::RayCast(start,end,stage.transform,stage.model.get(),hitPosition,hitNormal))
				{
					player.position.y=hitPosition.y;
					player.velocity.y=0.0f;
					moveY=0.0f;
					player.onGround = true;
				}
				
			}*/
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
		}

		// �ړ�
		player.position.x += moveX;
		player.position.y += moveY;
		player.position.z += moveZ;
		
	}

	// �v���C���[�s��X�V����
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(player.angle.x, player.angle.y, player.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
		DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
	}
	
}

// �`�揈��
void LandWalkScene::Render(float elapsedTime)
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

	// HP�Q�[�W�̕`��ʒu�ƃT�C�Y���v�Z
	//float barWidth = 30.0f; // HP�o�[�̕�
	//float barHeight = 5.0f; // HP�o�[�̍���

	// HP�o�[�̕`��
	/*sprite->Render(dc,
		10,
		10,
		0,
		barWidth,
		barHeight,
		0,
		1.0f, 0.0f, 0.0f, 1.0f);*/
}

// GUI�`�揈��
void LandWalkScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);

	if (ImGui::Begin(u8"�n����s", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"�ړ�����FWASD");
		ImGui::Text(u8"�W�����v����FSpace");
		if (ImGui::Button(u8"���Z�b�g"))
		{
			player.position = { 13, 5, 16 };
			player.velocity = { 0, 0, 0 };
		}

		ImGui::DragFloat3("Position", &player.position.x, 0.1f);
		ImGui::DragFloat3("Velocity", &player.velocity.x, 0.1f);
		ImGui::Checkbox("OnGround", &player.onGround);
		ImGui::DragFloat("Gravity", &gravity, 0.1f);
		ImGui::DragFloat("Acceleration", &acceleration, 0.1f);
		ImGui::DragFloat("Deceleration", &deceleration, 0.1f);
		ImGui::DragFloat("MoveSpeed", &moveSpeed, 0.1f);
		ImGui::DragFloat("TurnSpeed", &turnSpeed, 0.1f);
		ImGui::DragFloat("JumpSpeed", &jumpSpeed, 0.1f);
		ImGui::DragFloat("DownhillOffset", &downhillOffset, 0.01f);
	}
	ImGui::End();
}
