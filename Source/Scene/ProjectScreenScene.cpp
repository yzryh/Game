#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/ProjectScreenScene.h"

// コンストラクタ
ProjectScreenScene::ProjectScreenScene()
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
		{ 0, 30, 30 },		// 視点
		{ 0, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	sprite = std::make_unique<Sprite>(device);
	stage.model = std::make_unique<Model>("Data/Model/Stage/ExampleStage.mdl");
}

// 更新処理
void ProjectScreenScene::Update(float elapsedTime)
{
	
}

// 描画処理
void ProjectScreenScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	D3D11_VIEWPORT viewport;
	UINT numViewPorts = 1;
	dc->RSGetViewports(&numViewPorts, &viewport);
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// マウス左クリックした
	if (GetAsyncKeyState(VK_LBUTTON) & 0x01)
	{
		// スクリーンサイズ取得
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		// マウスカーソル位置の取得
		POINT cursor;
		::GetCursorPos(&cursor);
		::ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera.GetView());
		DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera.GetProjection());
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
		
		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		DirectX::XMFLOAT3 screenPosition;
		//マウスカーソル座標を取得
		screenPosition.x = static_cast<float> (cursor.x);
		screenPosition.y = static_cast<float> (cursor.y);

		//マウスカーソル座標を取得
		DirectX::XMFLOAT3 screenPositionA{};
		screenPositionA.x = static_cast<float> (cursor.x);
		screenPositionA.y = static_cast<float> (cursor.y);

		DirectX::XMVECTOR ScreenPositionA = DirectX::XMLoadFloat3(&screenPositionA);

		DirectX::XMFLOAT3 screenPositionB{};
		screenPositionB.x = static_cast<float> (cursor.x);
		screenPositionB.y = static_cast<float> (cursor.y);
		screenPositionB.z = 1.0f;

		DirectX::XMVECTOR ScreenPositionB = DirectX::XMLoadFloat3(&screenPositionB);
		//Z値が0.0の時のワールド座標
		//スクリーン座標をワールド座標に設定		
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
		//Z値が1.0の時のワールド座標
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

		DirectX::XMVECTOR Ray = DirectX::XMVectorSubtract(WorldPositionB, WorldPositionA);//ベクトルの長さ求める
		DirectX::XMVECTOR V = DirectX::XMVector3Normalize(Ray);//正規化
		DirectX::XMVECTOR Length = DirectX::XMVector3Length(V);

		DirectX::XMFLOAT3 start;
		DirectX::XMStoreFloat3(&start, WorldPositionA);
		DirectX::XMFLOAT3 end;
		DirectX::XMStoreFloat3(&end, WorldPositionB);
		//レイキャストで敵追加
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		if (Collision::RayCast(start, end, stage.transform, stage.model.get(), hitPosition, hitNormal))
		{
			Object& obj = objs.emplace_back();
			obj.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
			obj.scale = { 0.01f,0.01f,0.01f };
			obj.position = hitPosition;
	}
		

	// ステージ行列更新処理
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(stage.scale.x, stage.scale.y, stage.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(stage.angle.x, stage.angle.y, stage.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(stage.position.x, stage.position.y, stage.position.z);
		DirectX::XMStoreFloat4x4(&stage.transform, S * R * T);
	}

	// オブジェクト行列更新処理
	for (Object& obj : objs)
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(obj.angle.x, obj.angle.y, obj.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
		DirectX::XMStoreFloat4x4(&obj.transform, S * R * T);

		DirectX::XMFLOAT3 worldPosition = obj.position;
		worldPosition.y = obj.position.y + 2;
		DirectX::XMVECTOR EnemyWorldPosition = DirectX::XMLoadFloat3(&worldPosition);
		//ワールド座標をスクリーン座標に変換
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

		//スクリーン座標
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		// HPゲージの描画位置とサイズを計算
		float barWidth = 30.0f; // HPバーの幅
		float barHeight = 5.0f; // HPバーの高さ

		// HPバーの描画
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
	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, stage.transform, stage.model.get(), ShaderId::Lambert);

	// スクリーンサイズ取得
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

// GUI描画処理
void ProjectScreenScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"スクリーン座標変換", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"クリック：キャラ配置");
	}
	ImGui::End();
}
