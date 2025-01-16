#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/AttachWeaponScene.h"

// �R���X�g���N�^
AttachWeaponScene::AttachWeaponScene()
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
		{ 4, 2, 2 },		// ���_
		{ 0, 1.5f, 1 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	player.position = { 0, 0, 0 };
	player.scale = { 1.0f, 1.0f, 1.0f };
	player.model = std::make_unique<Model>("Data/Model/RPG-Character/RPG-Character.mdl");

	weapon.position = { -0.1f, 0.09f, -0.07f };
	weapon.angle = { 0.21f, -0.44f, 0.29f };
	weapon.scale = { 1.0f, 1.0f, 1.0f };
	weapon.model = std::make_unique<Model>("Data/Model/RPG-Character/2Hand-Sword.mdl");
}

// �X�V����
void AttachWeaponScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �v���C���[�s��X�V����
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(player.angle.x, player.angle.y, player.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
		DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
	}

	const char* rightHandName = "B_R_Hand";

	//����̃��[�J�����W
	
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(weapon.scale.x, weapon.scale.y, weapon.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(weapon.angle.x, weapon.angle.y, weapon.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(weapon.position.x, weapon.position.y, weapon.position.z);
		DirectX::XMMATRIX WeaponLoTr = S * R * T;
	

	for (const Model::Node& node : player.model->GetNodes())
	{
		if (::strcmp(node.name, rightHandName) == 0)
		{
			DirectX::XMMATRIX HandGlTr = DirectX::XMLoadFloat4x4(&node.globalTransform);
			DirectX::XMMATRIX PlayerWoTr = DirectX::XMLoadFloat4x4(&player.transform);
			DirectX::XMMATRIX WeaponWotr = WeaponLoTr * HandGlTr * PlayerWoTr;
			a = WeaponWotr;
			DirectX::XMStoreFloat4x4(&weapon.transform, WeaponWotr);
		}
	}
}

// �`�揈��
void AttachWeaponScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �����蔻��p�I�t�Z�b�g���g���A�����蔻��ʒu�����߂�
	
	DirectX::XMVECTOR P = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&weaponHitOffset),a);
	DirectX::XMFLOAT3 ps;
	DirectX::XMStoreFloat3(&ps, P);
	// �����蔻�苅�`��
	float weaponHitRadius = 0.5f;
	shapeRenderer->DrawSphere(ps,weaponHitRadius,{1,0,0,1});
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
	// ���f���`��
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, weapon.transform, weapon.model.get(), ShaderId::Lambert);

	// ���`��
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	primitiveRenderer->DrawAxis(weapon.transform, { 1,1,1,1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �O���b�h�`��
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI�`�揈��
void AttachWeaponScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(350, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"�A�^�b�`�����g", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::DragFloat3("PlayerPosition", &player.position.x, 0.01f);
		ImGui::DragFloat3("WeaponPosition", &weapon.position.x, 0.01f);
		ImGui::DragFloat3("WeaponAngle", &weapon.angle.x, 0.01f);
		ImGui::DragFloat3("WeaponHitOffset", &weaponHitOffset.x, 0.01f);
	}
	ImGui::End();
}
