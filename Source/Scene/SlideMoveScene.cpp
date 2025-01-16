#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/SlideMoveScene.h"

// コンストラクタ
SlideMoveScene::SlideMoveScene()
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
		{ 13, 10, 27 },		// 視点
		{ 13, 1, 16 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	// モデル
	stage.model = std::make_unique<Model>("Data/Model/Greybox/Greybox.mdl");

	player.position = { 13, 0, 16 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// 更新処理
void SlideMoveScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// プレイヤー移動処理
	{
		// 入力処理
		float axisX = 0.0f;
		float axisY = 0.0f;
		if (GetAsyncKeyState('W') & 0x8000) axisY += 1.0f;
		if (GetAsyncKeyState('S') & 0x8000) axisY -= 1.0f;
		if (GetAsyncKeyState('D') & 0x8000) axisX += 1.0f;
		if (GetAsyncKeyState('A') & 0x8000) axisX -= 1.0f;

		// カメラの方向
		const DirectX::XMFLOAT3& cameraFront = camera.GetFront();
		const DirectX::XMFLOAT3& camemraRight = camera.GetRight();
		float cameraFrontLengthXZ = sqrtf(cameraFront.x * cameraFront.x + cameraFront.z * cameraFront.z);
		float cameraRightLengthXZ = sqrtf(camemraRight.x * camemraRight.x + camemraRight.z * camemraRight.z);
		float cameraFrontX = cameraFront.x / cameraFrontLengthXZ;
		float cameraFrontZ = cameraFront.z / cameraFrontLengthXZ;
		float cameraRightX = camemraRight.x / cameraRightLengthXZ;
		float cameraRightZ = camemraRight.z / cameraRightLengthXZ;

		// 移動ベクトル
		const float speed = 5.0f * elapsedTime;
		float vecX = cameraFrontX * axisY + cameraRightX * axisX;
		float vecZ = cameraFrontZ * axisY + cameraRightZ * axisX;
		float moveX = vecX * speed;
		float moveZ = vecZ * speed;
		float moveLength = sqrtf(moveX * moveX + moveZ * moveZ);

		// 壁ずり移動処理
		if (moveLength > 0)
		{
			// レイの始点と終点を求める
			const DirectX::XMFLOAT3 s =
			{ player.position.x,
			  player.position.y + 0.5f,
			  player.position.z };
			const DirectX::XMFLOAT3 e = {
				player.position.x+moveX,
				player.position.y +0.5f,
				player.position.z+moveZ};
			// レイキャストを行い、交点を求める
			DirectX::XMFLOAT3 p, n;
			if (Collision::RayCast(s, e, stage.transform, stage.model.get(), p, n))
			{
				//交点から終点へのベクトルを求める
				DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&p);
				DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&e);
				DirectX::XMVECTOR PE = DirectX::XMVectorSubtract(E, P);

				//三角関数で終点から壁までの長さを求める
				DirectX::XMVECTOR N = DirectX::XMLoadFloat3(&n);
				DirectX::XMVECTOR A = DirectX::XMVector3Dot(DirectX::XMVectorNegate(PE), N);
				//壁までの長さを少しだけ長くなるように補正
				float a = DirectX::XMVectorGetX(A) + 0.001f;

				//壁ずりベクトルを求める
				DirectX::XMVECTOR R = DirectX::XMVectorAdd(PE, DirectX::XMVectorScale(N, a));

				//壁ずり後の位置を決める
				DirectX::XMVECTOR Q = DirectX::XMVectorAdd(P, R);
				DirectX::XMFLOAT3 q;
				DirectX::XMStoreFloat3(&q, Q);
				//壁際で壁ずり後の位置がめり込んでないかレイキャストでチェックする
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
			// 進行方向を向くようにする
			float vx = moveX / moveLength;
			float vz = moveZ / moveLength;
			player.angle.y = atan2f(vx, vz);
		}

		if (moveLength > 0)
			// 接地処理
		{
			// レイの始点と終点を求める
			const DirectX::XMFLOAT3 s = { player.position.x, player.position.y + 0.5f, player.position.z };
			const DirectX::XMFLOAT3 e = { player.position.x, player.position.y - 3.0f, player.position.z };

			// レイキャストを行い、交点を求める
			DirectX::XMFLOAT3 p, n;
			if (Collision::RayCast(s, e, stage.transform, stage.model.get(), p, n))
			{
				// 交点のY座標をプレイヤーに位置に設定する
				player.position.y = p.y;
			}
		}
		// プレイヤー行列更新処理
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(player.angle.x, player.angle.y, player.angle.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
			DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
		}
	}
}

// 描画処理
void SlideMoveScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, stage.transform, stage.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, player.transform, player.model.get(), ShaderId::Lambert);
}

// GUI描画処理
void SlideMoveScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"壁ずり移動", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"移動操作：WASD");

		ImGui::DragFloat3("Position", &player.position.x, 0.1f);
	}
	ImGui::End();
}
