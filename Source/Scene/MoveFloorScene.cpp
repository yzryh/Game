#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/MoveFloorScene.h"

// コンストラクタ
MoveFloorScene::MoveFloorScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// カメラ設定
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),	// 画角
		screenWidth / screenHeight,			// 画面アスペクト比
		0.1f,								// ニアクリップ
		1000.0f								// ファークリップ
	);
	camera.SetLookAt(
		{ 0, 10, 10 },		// 視点
		{ 0, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
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

// 更新処理
void MoveFloorScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// 移動床処理
	{
		float oldFloorAngle = floor.angle.y;
		// 移動＆回転処理
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
			// 床のワールド行列を更新する
			floor.UpdateTransform();
		}
	}

	// プレイヤー移動処理
	{
		// 移動入力処理
		const float speed = 3.0f * elapsedTime;
		DirectX::XMFLOAT3 vec = {};
		if (GetAsyncKeyState('W') & 0x8000) vec.z += speed;
		if (GetAsyncKeyState('S') & 0x8000) vec.z -= speed;
		if (GetAsyncKeyState('D') & 0x8000) vec.x += speed;
		if (GetAsyncKeyState('A') & 0x8000) vec.x -= speed;

		// カメラの向きを計算
		const DirectX::XMFLOAT3& front = camera.GetFront();
		const DirectX::XMFLOAT3& right = camera.GetRight();
		float frontLengthXZ = sqrtf(front.x * front.x + front.z * front.z);
		float rightLengthXZ = sqrtf(right.x * right.x + right.z * right.z);
		float frontX = front.x / frontLengthXZ;
		float frontZ = front.z / frontLengthXZ;
		float rightX = right.x / rightLengthXZ;
		float rightZ = right.z / rightLengthXZ;

		// 移動ベクトルを求める
		float moveX = frontX * vec.z + rightX * vec.x;
		float moveZ = frontZ * vec.z + rightZ * vec.x;
		float moveLength = sqrtf(moveX * moveX + moveZ * moveZ);

		// 移動
		player.position.x += moveX;
		player.position.z += moveZ;

		// 接地処理
		{
			const DirectX::XMFLOAT3 worldStart = { player.position.x, player.position.y + 2.0f, player.position.z };
			const DirectX::XMFLOAT3 worldEnd = { player.position.x, player.position.y - 3.0f, player.position.z };

			DirectX::XMFLOAT3 hitPosition, hitNormal;
			if (Collision::RayCast(worldStart, worldEnd, floor.transform, floor.model.get(), hitPosition, hitNormal))
			{
				player.position.y = hitPosition.y;
			}
		}

		// プレイヤーのワールド行列を更新する
		player.UpdateTransform();
	}
}

// 描画処理
void MoveFloorScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, floor.transform, floor.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// グリッド描画
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI描画処理
void MoveFloorScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"移動床", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"キャラ移動操作：WASD");
		ImGui::Text(u8"床移動操作：方向キー");		
		ImGui::Text(u8"床回転操作：スペースキー");
	}
	ImGui::End();
}
