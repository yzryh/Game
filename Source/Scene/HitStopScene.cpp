#include <imgui.h>
#include <ImGuizmo.h>
#include <DirectXCollision.h>
#include "Graphics.h"
#include "Collision.h"
#include "Scene/HitStopScene.h"

// コンストラクタ
HitStopScene::HitStopScene()
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
		{ 4, 2, 2 },		// 視点
		{ 0, 1.5f, 1 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	character.position = { 0, 0, 0 };
	character.scale = { 0.01f, 0.01f, 0.01f };
	character.model = std::make_unique<Model>("Data/Model/Mr.Incredible/Mr.Incredible.mdl");

	weapon.position = { 0, 1, 1.5f };
	weapon.angle = { 0, 0, 0 };
	weapon.scale = { 1.0f, 1.0f, 1.0f };
	weapon.model = std::make_unique<Model>("Data/Model/RPG-Character/2Hand-Sword.mdl");
}

// 更新処理
void HitStopScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	//剣とキャラクターの衝突判定
	{
		DirectX::XMFLOAT3 p1, p2;
		p1.x = character.position.x;
		p1.y = character.position.y + characterHitOffset;
		p1.z = character.position.z;
		p2.x = weapon.transform._41 + weapon.transform._31 * weaponHitOffset;
		p2.y = weapon.transform._42 + weapon.transform._32 * weaponHitOffset;
		p2.z = weapon.transform._43 + weapon.transform._33 * weaponHitOffset;

		float vx = p1.x - p2.x;
		float vy = p1.y - p2.y;
		float vz = p1.z - p2.z;
		float len = sqrtf(vx * vx + vy * vy + vz * vz);
		float range = characterHitOffset + weaponHitRadius;
		if (len < range)
		{
			if (!hitStop)
			{
				hitStop = true;
				hitStopLastSeconds = hitStopSecondsLength;
			}
		}
	}
	float timeScale = 1.0f;
	if (hitStop)
	{
		hitStopLastSeconds -= elapsedTime;
		if (hitStopLastSeconds <= 0)
		{
			hitStopLastSeconds = 0;
			hitStop = !hitStop;
		}
		
		if (hitStop)
		{
			timeScale = 1.0f - (hitStopLastSeconds / hitStopSecondsLength);
		}
		//カメラシェイク
		{
			auto getRand = []()->float
				{
					const int max = RAND_MAX / 2;
					return static_cast<float>(rand() - max) / max;
				};
			float t = hitStopLastSeconds / hitStopSecondsLength;

			DirectX::XMFLOAT3 shake;
			shake.x = getRand() * cameraShakeRange * t;
			shake.y = getRand() * cameraShakeRange * t;
			shake.z = getRand() * cameraShakeRange * t;

			DirectX::XMFLOAT3 focus = camera.GetFocus();
			focus.x += shake.x;
			focus.y += shake.y;
			focus.z += shake.z;
			camera.SetLookAt(camera.GetEye(), focus, camera.GetUp());
		}
	}


		// 剣の動き
		weaponMotionCurrentSeconds += elapsedTime * timeScale;
		if (weaponMotionCurrentSeconds > weaponMotionSecondsLength)
		{
			weaponMotionCurrentSeconds = 0.0f;
		}
		weapon.angle.y = Easing(weaponMotionCurrentSeconds / weaponMotionSecondsLength) * DirectX::XM_2PI;

		// キャラクター行列更新処理
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(character.scale.x, character.scale.y, character.scale.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(character.angle.x, character.angle.y, character.angle.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(character.position.x, character.position.y, character.position.z);
			DirectX::XMStoreFloat4x4(&character.transform, S * R * T);
		}

		// 武器行列更新処理
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(weapon.scale.x, weapon.scale.y, weapon.scale.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(weapon.angle.x, weapon.angle.y, weapon.angle.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(weapon.position.x, weapon.position.y, weapon.position.z);
			DirectX::XMStoreFloat4x4(&weapon.transform, S * R * T);
		}

	}

// 描画処理
void HitStopScene::Render(float elapsedTime)
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
	modelRenderer->Render(rc, character.transform, character.model.get(), ShaderId::Lambert);
	modelRenderer->Render(rc, weapon.transform, weapon.model.get(), ShaderId::Lambert);

	DirectX::XMFLOAT3 p;
	p.x = weapon.transform._41 + weapon.transform._31 * weaponHitOffset;
	p.y = weapon.transform._42 + weapon.transform._32 * weaponHitOffset;
	p.z = weapon.transform._43 + weapon.transform._33 * weaponHitOffset;
	shapeRenderer->DrawSphere(p, weaponHitRadius, { 1,1,1,1 });

	p.x = character.position.x;
	p.y = character.position.y + characterHitOffset;
	p.z = character.position.z;
	shapeRenderer->DrawSphere(p, characterHitRadius, { 1,1,1,1 });
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());

	// 軸描画
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	primitiveRenderer->DrawAxis(weapon.transform, { 1,1,1,1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// グリッド描画
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI描画処理
void HitStopScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"ヒットストップ", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::DragFloat3("CharacterPosition", &character.position.x, 0.01f);
		ImGui::DragFloat3("WeaponPosition", &weapon.position.x, 0.01f);
		ImGui::DragFloat3("WeaponAngle", &weapon.angle.x, 0.01f);

		ImGui::DragFloat("weaonHitOffset", &weaponHitOffset, 0.01f);
		ImGui::DragFloat("weaonHitRadius", &weaponHitRadius, 0.01f);
	}
	ImGui::End();
}
