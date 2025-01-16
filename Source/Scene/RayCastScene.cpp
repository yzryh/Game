#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/RayCastScene.h"

// �R���X�g���N�^
RayCastScene::RayCastScene()
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
		{ -4, 4, 10 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// ���f��
	model = std::make_unique<Model>("Data/Model/Cube/cube.003.1.mdl");

	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0, 0.8f, 0.0f);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(-1, 0, -1);
	DirectX::XMStoreFloat4x4(&worldTransform, S * R * T);
}

// �X�V����
void RayCastScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

}

// �`�揈��
void RayCastScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �O�p�`���_
	DirectX::XMFLOAT3 vertices1[3] = {
		{ -3, 3, 0 },
		{ 0, 2, 3 },
		{ 3, 3, 0 },
	};
	DirectX::XMFLOAT4 color1 = { 0, 1, 0, 1 };

	DirectX::XMFLOAT3 vertices2[3] = {
		{ -2, 2, 0 },
		{ 2, 2, 0 },
		{ 0, 1, 2 },
	};
	DirectX::XMFLOAT4 color2 = { 0, 1, 0, 1 };

	// ���C�̎n�_�ƏI�_
	const DirectX::XMFLOAT3 startPosition = { 2, 4, 3 };
	static DirectX::XMFLOAT3 endPosition = { -1.7f, 0, -1.7f };
	{
		// �M�Y���ŏI�_�ʒu��ҏW
		const DirectX::XMFLOAT4X4& view = camera.GetView();
		const DirectX::XMFLOAT4X4& projection = camera.GetProjection();
		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixTranslation(endPosition.x, endPosition.y, endPosition.z));
		if (ImGuizmo::Manipulate(
			&view._11, &projection._11,
			ImGuizmo::TRANSLATE,
			ImGuizmo::WORLD,
			&world._11,
			nullptr))
		{
			endPosition.x = world._41;
			endPosition.y = world._42;
			endPosition.z = world._43;
		}
	}

	// �O�p�`�Ƀ��C�L���X�g
	{
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		// ��ڂ̎O�p�`
		if (RayCast(startPosition, endPosition, vertices1[0], vertices1[1], vertices1[2], hitPosition, hitNormal))
		{
			// ���������ʒu�Ɩ@����\��
			shapeRenderer->DrawSphere(hitPosition, 0.1f, { 1, 1, 0, 1 });

			DirectX::XMFLOAT3 p = hitPosition;
			p.x += hitNormal.x * 1.0f;
			p.y += hitNormal.y * 1.0f;
			p.z += hitNormal.z * 1.0f;
			primitiveRenderer->AddVertex(hitPosition, { 1, 1, 0, 1 });
			primitiveRenderer->AddVertex(p, { 1, 1, 0, 1 });
			primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			color1 = { 1, 0, 0, 1 };
		}
		// ��ڂ̎O�p�`
		if (RayCast(startPosition, endPosition, vertices2[0], vertices2[1], vertices2[2], hitPosition, hitNormal))
		{
			// ���������ʒu�Ɩ@����\��
			shapeRenderer->DrawSphere(hitPosition, 0.1f, { 1, 1, 0, 1 });

			DirectX::XMFLOAT3 p = hitPosition;
			p.x += hitNormal.x * 1.0f;
			p.y += hitNormal.y * 1.0f;
			p.z += hitNormal.z * 1.0f;
			primitiveRenderer->AddVertex(hitPosition, { 1, 1, 0, 1 });
			primitiveRenderer->AddVertex(p, { 1, 1, 0, 1 });
			primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			color2 = { 1, 0, 0, 1 };
		}
	}
	// ���f���Ƀ��C�L���X�g
	{
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		if (Collision::RayCast(startPosition, endPosition, worldTransform, model.get(), hitPosition, hitNormal))
		{
			// ���������ʒu�Ɩ@����\��
			shapeRenderer->DrawSphere(hitPosition, 0.1f, { 1, 1, 0, 1 });

			DirectX::XMFLOAT3 p = hitPosition;
			p.x += hitNormal.x * 1.0f;
			p.y += hitNormal.y * 1.0f;
			p.z += hitNormal.z * 1.0f;
			primitiveRenderer->AddVertex(hitPosition, { 1, 1, 0, 1 });
			primitiveRenderer->AddVertex(p, { 1, 1, 0, 1 });
			primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		}
	}

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// �O�p�`�|���S���`��
	primitiveRenderer->AddVertex(vertices1[0], color1);
	primitiveRenderer->AddVertex(vertices1[1], color1);
	primitiveRenderer->AddVertex(vertices1[2], color1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	primitiveRenderer->AddVertex(vertices2[0], color2);
	primitiveRenderer->AddVertex(vertices2[1], color2);
	primitiveRenderer->AddVertex(vertices2[2], color2);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O�p�`�G�b�W�`��
	primitiveRenderer->AddVertex(vertices1[0], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices1[1], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices1[2], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices1[0], { 0, 0, 1, 1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	primitiveRenderer->AddVertex(vertices2[0], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices2[1], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices2[2], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(vertices2[0], { 0, 0, 1, 1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// ���C�`��
	primitiveRenderer->AddVertex(startPosition, { 0, 1, 1, 1 });
	primitiveRenderer->AddVertex(endPosition, { 0, 1, 1, 1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �V�F�C�v�`��
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());

	// ���f���`��
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, worldTransform, model.get(), ShaderId::Lambert);
}

// GUI�`�揈��
void RayCastScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"���C�L���X�g", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"�M�Y���Ń��C�̏I�_�ʒu�𑀍�");
		ImGui::Text(u8"�E�N���b�N�{�}�E�X����ŃJ������]");
		ImGui::Text(u8"���N���b�N�{�}�E�X����ŃJ�����ړ�");
		ImGui::Text(u8"�}�E�X�z�C�[���ŃJ�����Y�[��");
	}
	ImGui::End();
}

// ���C�L���X�g
bool RayCastScene::RayCast(				// ����������true��Ԃ�
	const DirectX::XMFLOAT3& start,		// ���C�̎n�_
	const DirectX::XMFLOAT3& end,		// ���C�̏I�_
	const DirectX::XMFLOAT3& vertexA,	// �O�p�`�̈�ڂ̒��_���W
	const DirectX::XMFLOAT3& vertexB,	// �O�p�`�̓�ڂ̒��_���W
	const DirectX::XMFLOAT3& vertexC,	// �O�p�`�̎O�ڂ̒��_���W
	DirectX::XMFLOAT3& hitPosition,		// ��_�̊i�[��
	DirectX::XMFLOAT3& hitNormal)		// �@���̊i�[��
{
	//�n�_�ƏI�_���烌�C�̒P�ʃx�N�g���ƒ��������߂�
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Start,End);
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(Vec);
	DirectX::XMVECTOR Length = DirectX::XMVector3Length(Vec);
	float distance = DirectX::XMVectorGetX(Length);

	//�O�p�`�̒��_���W
	DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&vertexA);
	DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&vertexB);
	DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&vertexC);
	if (DirectX::TriangleTests::Intersects(A,B,C,Direction))
	{
		C = DirectX::XMVector3Cross(A, B);
		C = DirectX::XMVector3Normalize(C);
		//�\������
		DirectX::XMVECTOR D = DirectX::XMVector3Dot(Direction, C);
		DirectX::XMVECTOR SA = DirectX::XMVectorSubtract(A, Vec);
		DirectX::XMVECTOR X = DirectX::XMVectorDivide
		(DirectX::XMVector3Dot(SA, C), D);
		float d;
		DirectX::XMStoreFloat(&d, D);

		if (d >= 0.0f)
			return true;
		
		return true;
	}
	return false;
}
