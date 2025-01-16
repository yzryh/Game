#include <imgui.h>
#include "Graphics.h"
#include "ResourceManager.h"
#include "Scene/ResourceManagementScene.h"

// コンストラクタ
ResourceManagementScene::ResourceManagementScene()
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
		{ 0, 25, 25 },		// 視点
		{ 0, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);
}

// 更新処理
void ResourceManagementScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// オブジェクト行列更新処理
	for (Object& obj : objs)
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(obj.scale.x, obj.scale.y, obj.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(obj.angle.x, obj.angle.y, obj.angle.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
		DirectX::XMStoreFloat4x4(&obj.transform, S * R * T);
	}
}

// 描画処理
void ResourceManagementScene::Render(float elapsedTime)
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


	for (const Object& obj : objs)
	{
		modelRenderer->Render(rc, obj.transform, obj.model.get(), ShaderId::Lambert);
	}

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// グリッド描画
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI描画処理
void ResourceManagementScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"リソース管理", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		if (ImGui::Button("Load"))
		{
			float start = timer.GetMilliseconds();
			objs.clear();

			for (int x = -10; x < 10; ++x)
			{
				for (int z = -10; z < 10; ++z)
				{
					Object& obj = objs.emplace_back();
					obj.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");
					obj.scale = { 0.01f, 0.01f, 0.01f };
					obj.position.x = 0.5f + static_cast<float>(x);
					obj.position.y = 0.0f;
					obj.position.z = 0.5f + static_cast<float>(z);
				}
			}
			float end = timer.GetMilliseconds();
			loadTime = end - start;
		}
		if (ImGui::Button("Clear"))
		{
			objs.clear();
		}
		ImGui::Text("Loading Time:%.2f (ms)", loadTime);

		ResourceManager::Instance().DrawDebugGUI();
	}
	ImGui::End();
}

