#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/ProjectScreenScene.h"

// �R���X�g���N�^
ProjectScreenScene::ProjectScreenScene()
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
		{ 0, 30, 30 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	sprite = std::make_unique<Sprite>(device);
	stage.model = std::make_unique<Model>("Data/Model/Stage/ExampleStage.mdl");
}

// �X�V����
void ProjectScreenScene::Update(float elapsedTime)
{
	
}

// �`�揈��
void ProjectScreenScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	D3D11_VIEWPORT viewport;
	UINT numViewPorts = 1;
	dc->RSGetViewports(&numViewPorts, &viewport);
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �}�E�X���N���b�N����
	if (GetAsyncKeyState(VK_LBUTTON) & 0x01)
	{
		// �X�N���[���T�C�Y�擾
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		// �}�E�X�J�[�\���ʒu�̎擾
		POINT cursor;
		::GetCursorPos(&cursor);
		::ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera.GetView());
		DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera.GetProjection());
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
		
		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		DirectX::XMFLOAT3 screenPosition;
		//�}�E�X�J�[�\�����W���擾
		screenPosition.x = static_cast<float> (cursor.x);
		screenPosition.y = static_cast<float> (cursor.y);

		//�}�E�X�J�[�\�����W���擾
		DirectX::XMFLOAT3 screenPositionA{};
		screenPositionA.x = static_cast<float> (cursor.x);
		screenPositionA.y = static_cast<float> (cursor.y);

		DirectX::XMVECTOR ScreenPositionA = DirectX::XMLoadFloat3(&screenPositionA);

		DirectX::XMFLOAT3 screenPositionB{};
		screenPositionB.x = static_cast<float> (cursor.x);
		screenPositionB.y = static_cast<float> (cursor.y);
		screenPositionB.z = 1.0f;

		DirectX::XMVECTOR ScreenPositionB = DirectX::XMLoadFloat3(&screenPositionB);
		//Z�l��0.0�̎��̃��[���h���W
		//�X�N���[�����W�����[���h���W�ɐݒ�		
		DirectX::XMVECTOR WorldPositionA = DirectX::XMVector3Unproject(
			ScreenPositionA,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);
		//Z�l��1.0�̎��̃��[���h���W
		DirectX::XMVECTOR WorldPositionB = DirectX::XMVector3Unproject(
			ScreenPositionB,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		DirectX::XMVECTOR Ray = DirectX::XMVectorSubtract(WorldPositionB, WorldPositionA);//�x�N�g���̒������߂�
		DirectX::XMVECTOR V = DirectX::XMVector3Normalize(Ray);//���K��
		DirectX::XMVECTOR Length = DirectX::XMVector3Length(V);

		DirectX::XMFLOAT3 start;
		DirectX::XMStoreFloat3(&start, WorldPositionA);
		DirectX::XMFLOAT3 end;
		DirectX::XMStoreFloat3(&end, WorldPositionB);
		//���C�L���X�g�œG�ǉ�
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		if (Collision::RayCast(start, end, stage.transform, stage.model.get(), hitPosition, hitNormal))
		{
			Object& obj = objs.emplace_back();
			obj.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
			obj.scale = { 0.01f,0.01f,0.01f };
			obj.position = hitPosition;
	}
		

	// �X�e�[�W�s��X�V����
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(stage.scale.x, stage.scale.y, stage.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(stage.angle.x, stage.angle.y, stage.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(stage.position.x, stage.position.y, stage.position.z);
		DirectX::XMStoreFloat4x4(&stage.transform, S * R * T);
	}

	// �I�u�W�F�N�g�s��X�V����
	for (Object& obj : objs)
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(obj.angle.x, obj.angle.y, obj.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
		DirectX::XMStoreFloat4x4(&obj.transform, S * R * T);

		DirectX::XMFLOAT3 worldPosition = obj.position;
		worldPosition.y = obj.position.y + 2;
		DirectX::XMVECTOR EnemyWorldPosition = DirectX::XMLoadFloat3(&worldPosition);
		//���[���h���W���X�N���[�����W�ɕϊ�
		DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
			EnemyWorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		//�X�N���[�����W
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		// HP�Q�[�W�̕`��ʒu�ƃT�C�Y���v�Z
		float barWidth = 30.0f; // HP�o�[�̕�
		float barHeight = 5.0f; // HP�o�[�̍���

		// HP�o�[�̕`��
		sprite->Render(dc,
			screenPosition.x - barWidth * 0.5f,
			screenPosition.y - barHeight,
			obj.position.z,
			barWidth,
			barHeight,
			0,
			1.0f, 0.0f, 0.0f, 1.0f);
	}
	}
	// ���f���`��
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, stage.transform, stage.model.get(), ShaderId::Lambert);

	// �X�N���[���T�C�Y�擾
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera.GetView());
	DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera.GetProjection());
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	for (const Object& obj : objs)
	{
		modelRenderer->Render(rc, obj.transform, obj.model.get(), ShaderId::Lambert);
	}
}

// GUI�`�揈��
void ProjectScreenScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"�X�N���[�����W�ϊ�", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"�N���b�N�F�L�����z�u");
	}
	ImGui::End();
}
