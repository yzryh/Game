#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/TerrainAlignScene.h"

// コンストラクタ
TerrainAlignScene::TerrainAlignScene()
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
		{ -10, 10, 10 },		// 視点
		{ -10, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	floor.model = std::make_unique<Model>("Data/Model/Stage/ExampleStage.mdl");
	floor.position = { 0, 0, 0 };
	floor.angle = { 0, 0, 0 };
	floor.scale = { 5, 0.5f, 5 };

	player.position = { -10, 0, 0 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// 更新処理
void TerrainAlignScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

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

		// 地面の法線ベクトルを使用してキャラクターを地面に沿うようにする
		{
		}

		// 進行方向を向くY軸の回転値を求める
		if (moveLength > 0)
		{
			float vx = moveX / moveLength;
			float vz = moveZ / moveLength;
			player.angle.y = atan2f(vx, vz);
		}

	}
	{
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
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		if (Collision::RayCast(worldStart, worldEnd, floor.transform, floor.model.get(), hitPosition, hitNormal))
		{
			player.position.y = hitPosition.y;


			float ax=atan2f(hitNormal.z, hitNormal.y);
			float az = -atan2f(hitNormal.x, hitNormal.y);

			auto lerp = [](float v1, float v2, float t)->float
				{
					return(1.0f - t) * (v1)+t * (v2);
				};
			float t = 10 * elapsedTime;
			player.angle.x = lerp(player.angle.x, ax, t);
			player.angle.z = lerp(player.angle.z, az, t);

		}
	}

	// プレイヤー行列更新処理
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
		DirectX::XMMATRIX Y= DirectX::XMMatrixRotationY(player.angle.y);;
		DirectX::XMMATRIX X= DirectX::XMMatrixRotationX(player.angle.x);;
		DirectX::XMMATRIX Z = DirectX::XMMatrixRotationZ(player.angle.z);
		DirectX::XMMATRIX R = Y*X*Z;
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
		DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
	}

}

// 描画処理
void TerrainAlignScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, floor.transform, floor.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);
}

// GUI描画処理
void TerrainAlignScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"地形に沿う姿勢制御", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"移動操作：WASD");
	}
	ImGui::End();
}
