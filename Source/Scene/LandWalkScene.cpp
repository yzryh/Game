#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/LandWalkScene.h"

// コンストラクタ
LandWalkScene::LandWalkScene()
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

	player.position = { 13, 5, 16 };
	player.scale = { 0.01f, 0.01f, 0.01f };
	player.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
}

// 更新処理
void LandWalkScene::Update(float elapsedTime)
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
		if (GetAsyncKeyState(VK_SPACE) & 0x01) player.velocity.y = jumpSpeed;

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
		float vecX = cameraFrontX * axisY + cameraRightX * axisX;
		float vecZ = cameraFrontZ * axisY + cameraRightZ * axisX;
		float vecLength = sqrtf(vecX * vecX + vecZ * vecZ);

		// 横方向移動処理
		if (vecLength > 0)
		{
			// 単位ベクトル化
			vecX /= vecLength;
			vecZ /= vecLength;

			// 加速処理
			float acceleration = this->acceleration * elapsedTime;
			player.velocity.x += vecX * acceleration;
			player.velocity.z += vecZ * acceleration;

			// 最大速度制限
			float velocityLength = sqrtf(player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z);
			if (velocityLength > moveSpeed)
			{
				player.velocity.x = (player.velocity.x / velocityLength) * moveSpeed;
				player.velocity.z = (player.velocity.z / velocityLength) * moveSpeed;
			}

			// 進行方向を向くようにする
			{
				// プレイヤーの向いている方向
				float frontX = sinf(player.angle.y);
				float frontZ = cosf(player.angle.y);

				// 回転量調整
				float dot = frontX * vecX + frontZ * vecZ;
				float rot = (std::min)(1.0f - dot, turnSpeed * elapsedTime);

				// 左右判定をして回転処理
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
			// 減速処理
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

		// 重力処理
		player.velocity.y -= gravity * elapsedTime;

		// 移動量
		float moveX = player.velocity.x * elapsedTime;
		float moveY = player.velocity.y * elapsedTime;
		float moveZ = player.velocity.z * elapsedTime;
		
		// 接地処理
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
		}

		// 移動
		player.position.x += moveX;
		player.position.y += moveY;
		player.position.z += moveZ;
		
	}

	// プレイヤー行列更新処理
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(player.scale.x, player.scale.y, player.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(player.angle.x, player.angle.y, player.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(player.position.x, player.position.y, player.position.z);
		DirectX::XMStoreFloat4x4(&player.transform, S * R * T);
	}
	
}

// 描画処理
void LandWalkScene::Render(float elapsedTime)
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

	// HPゲージの描画位置とサイズを計算
	//float barWidth = 30.0f; // HPバーの幅
	//float barHeight = 5.0f; // HPバーの高さ

	// HPバーの描画
	/*sprite->Render(dc,
		10,
		10,
		0,
		barWidth,
		barHeight,
		0,
		1.0f, 0.0f, 0.0f, 1.0f);*/
}

// GUI描画処理
void LandWalkScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);

	if (ImGui::Begin(u8"地上歩行", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"移動操作：WASD");
		ImGui::Text(u8"ジャンプ操作：Space");
		if (ImGui::Button(u8"リセット"))
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
