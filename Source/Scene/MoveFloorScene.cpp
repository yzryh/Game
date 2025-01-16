#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/MoveFloorScene.h"

// �R���X�g���N�^
MoveFloorScene::MoveFloorScene()
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
		{ 0, 10, 10 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	floor.model = std::make_unique<Model>("Data/Model/Cube/Cube.mdl");
	floor.position = { 0, 0, 0 };
	floor.angle = { 0, 0, 0 };
	floor.scale = { 5, 0.5f, 5 };

	player.position = { 0, 0, 0 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// �X�V����
void MoveFloorScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �ړ�������
	{
		float oldFloorAngle = floor.angle.y;
		// �ړ�����]����
		const float speed = 3.0f * elapsedTime;
		if (GetAsyncKeyState(VK_UP) & 0x8000) floor.position.z -= speed;
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) floor.position.z += speed;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) floor.position.x -= speed;
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) floor.position.x += speed;
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) floor.angle.y += 0.5f * elapsedTime;

		const DirectX::XMFLOAT3 worldStart = {
			player.position.x,
			player.position.y + 2.0f,
			player.position.z
		};
		const DirectX::XMFLOAT3 worldEnd = {
			player.position.x,
			player.position.y - 3.0f,
			player.position.z
		};

		DirectX::XMFLOAT3 worldHitPosition, worldHitNormal;
		bool hit = Collision::RayCast(worldStart, worldEnd, floor.transform, floor.model.get(), worldHitPosition, worldHitNormal);
		if (hit)
		{
			DirectX::XMMATRIX floorIM;
			DirectX::XMMATRIX floorXM=DirectX::XMLoadFloat4x4(&floor.transform);
			floorIM = DirectX::XMMatrixInverse(nullptr,floorXM);
			DirectX::XMVECTOR WorldHitpos=DirectX::XMLoadFloat3(&worldHitPosition);
			DirectX::XMVECTOR LocalHitPos = DirectX::XMVector3Transform(WorldHitpos,floorIM);
			

			floor.UpdateTransform();
			DirectX::XMMATRIX floorWP = DirectX::XMLoadFloat4x4(&floor.transform);
			WorldHitpos = DirectX::XMVector3Transform(LocalHitPos, floorWP);
			DirectX::XMStoreFloat3(&player.position, WorldHitpos);

			player.angle.y += floor.angle.y - oldFloorAngle;
		}
		else
		{
			// ���̃��[���h�s����X�V����
			floor.UpdateTransform();
		}
	}

	// �v���C���[�ړ�����
	{
		// �ړ����͏���
		const float speed = 3.0f * elapsedTime;
		DirectX::XMFLOAT3 vec = {};
		if (GetAsyncKeyState('W') & 0x8000) vec.z += speed;
		if (GetAsyncKeyState('S') & 0x8000) vec.z -= speed;
		if (GetAsyncKeyState('D') & 0x8000) vec.x += speed;
		if (GetAsyncKeyState('A') & 0x8000) vec.x -= speed;

		// �J�����̌������v�Z
		const DirectX::XMFLOAT3& front = camera.GetFront();
		const DirectX::XMFLOAT3& right = camera.GetRight();
		float frontLengthXZ = sqrtf(front.x * front.x + front.z * front.z);
		float rightLengthXZ = sqrtf(right.x * right.x + right.z * right.z);
		float frontX = front.x / frontLengthXZ;
		float frontZ = front.z / frontLengthXZ;
		float rightX = right.x / rightLengthXZ;
		float rightZ = right.z / rightLengthXZ;

		// �ړ��x�N�g�������߂�
		float moveX = frontX * vec.z + rightX * vec.x;
		float moveZ = frontZ * vec.z + rightZ * vec.x;
		float moveLength = sqrtf(moveX * moveX + moveZ * moveZ);

		// �ړ�
		player.position.x += moveX;
		player.position.z += moveZ;

		// �ڒn����
		{
			const DirectX::XMFLOAT3 worldStart = { player.position.x, player.position.y + 2.0f, player.position.z };
			const DirectX::XMFLOAT3 worldEnd = { player.position.x, player.position.y - 3.0f, player.position.z };

			DirectX::XMFLOAT3 hitPosition, hitNormal;
			if (Collision::RayCast(worldStart, worldEnd, floor.transform, floor.model.get(), hitPosition, hitNormal))
			{
				player.position.y = hitPosition.y;
			}
		}

		// �v���C���[�̃��[���h�s����X�V����
		player.UpdateTransform();
	}
}

// �`�揈��
void MoveFloorScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// ���f���`��
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, floor.transform, floor.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI�`�揈��
void MoveFloorScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"�ړ���", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"�L�����ړ�����FWASD");
		ImGui::Text(u8"���ړ�����F�����L�[");		
		ImGui::Text(u8"����]����F�X�y�[�X�L�[");
	}
	ImGui::End();
}
