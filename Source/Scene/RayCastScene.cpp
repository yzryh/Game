#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/RayCastScene.h"

// コンストラクタ
RayCastScene::RayCastScene()
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
		{ -4, 4, 10 },		// 視点
		{ 0, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	// モデル
	model = std::make_unique<Model>("Data/Model/Cube/cube.003.1.mdl");

	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(0, 0.8f, 0.0f);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(-1, 0, -1);
	DirectX::XMStoreFloat4x4(&worldTransform, S * R * T);
}

// 更新処理
void RayCastScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

}

// 描画処理
void RayCastScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// 三角形頂点
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

	// レイの始点と終点
	const DirectX::XMFLOAT3 startPosition = { 2, 4, 3 };
	static DirectX::XMFLOAT3 endPosition = { -1.7f, 0, -1.7f };
	{
		// ギズモで終点位置を編集
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

	// 三角形にレイキャスト
	{
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		// 一つ目の三角形
		if (RayCast(startPosition, endPosition, vertices1[0], vertices1[1], vertices1[2], hitPosition, hitNormal))
		{
			// 交差した位置と法線を表示
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
		// 二つ目の三角形
		if (RayCast(startPosition, endPosition, vertices2[0], vertices2[1], vertices2[2], hitPosition, hitNormal))
		{
			// 交差した位置と法線を表示
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
	// モデルにレイキャスト
	{
		DirectX::XMFLOAT3 hitPosition, hitNormal;
		if (Collision::RayCast(startPosition, endPosition, worldTransform, model.get(), hitPosition, hitNormal))
		{
			// 交差した位置と法線を表示
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

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// 三角形ポリゴン描画
	primitiveRenderer->AddVertex(vertices1[0], color1);
	primitiveRenderer->AddVertex(vertices1[1], color1);
	primitiveRenderer->AddVertex(vertices1[2], color1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	primitiveRenderer->AddVertex(vertices2[0], color2);
	primitiveRenderer->AddVertex(vertices2[1], color2);
	primitiveRenderer->AddVertex(vertices2[2], color2);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// 三角形エッジ描画
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

	// レイ描画
	primitiveRenderer->AddVertex(startPosition, { 0, 1, 1, 1 });
	primitiveRenderer->AddVertex(endPosition, { 0, 1, 1, 1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// グリッド描画
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// シェイプ描画
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());

	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, worldTransform, model.get(), ShaderId::Lambert);
}

// GUI描画処理
void RayCastScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"レイキャスト", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::Text(u8"ギズモでレイの終点位置を操作");
		ImGui::Text(u8"右クリック＋マウス操作でカメラ回転");
		ImGui::Text(u8"中クリック＋マウス操作でカメラ移動");
		ImGui::Text(u8"マウスホイールでカメラズーム");
	}
	ImGui::End();
}

// レイキャスト
bool RayCastScene::RayCast(				// 交差したらtrueを返す
	const DirectX::XMFLOAT3& start,		// レイの始点
	const DirectX::XMFLOAT3& end,		// レイの終点
	const DirectX::XMFLOAT3& vertexA,	// 三角形の一つ目の頂点座標
	const DirectX::XMFLOAT3& vertexB,	// 三角形の二つ目の頂点座標
	const DirectX::XMFLOAT3& vertexC,	// 三角形の三つ目の頂点座標
	DirectX::XMFLOAT3& hitPosition,		// 交点の格納先
	DirectX::XMFLOAT3& hitNormal)		// 法線の格納先
{
	//始点と終点からレイの単位ベクトルと長さを求める
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Start,End);
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(Vec);
	DirectX::XMVECTOR Length = DirectX::XMVector3Length(Vec);
	float distance = DirectX::XMVectorGetX(Length);

	//三角形の頂点座標
	DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&vertexA);
	DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&vertexB);
	DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&vertexC);
	if (DirectX::TriangleTests::Intersects(A,B,C,Direction))
	{
		C = DirectX::XMVector3Cross(A, B);
		C = DirectX::XMVector3Normalize(C);
		//表裏判定
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
