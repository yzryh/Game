#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/AnimationScene.h"

// コンストラクタ
AnimationScene::AnimationScene()
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
		{ 0, 5, 7 },		// 視点
		{ 0, 0, 0 },		// 注視点
		{ 0, 1, 0 }			// 上ベクトル
	);
	cameraController.SyncCameraToController(camera);

	model = std::make_unique<Model>("Data/Model/Jammo/Jammo.mdl");
}

// 更新処理
void AnimationScene::Update(float elapsedTime)
{
	// カメラ更新処理
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);
	
	// ステート処理
	switch (state)
	{
		
		case State::Idle:
		{
			if (InputMove())
			{
				state = State::Run;
				PlayAnimation("Running", true);
			}
			if (InputJump())
			{
				state = State::Jump;
				PlayAnimation("Jump", false);
			}
			break;
		}
		case State::Run:
		{
			if (!InputMove())
			{
				state = State::Idle;
				PlayAnimation("Idle", true);
			}
			if (InputJump())
			{
				state = State::Jump;
				PlayAnimation("Jump",false);
			}
			break;
		}

		case State::Jump:
		{
			if (onGround)
			{
				if (!InputMove())
				{
					state = State::Idle;
					PlayAnimation("Idle", true);
				}
				if (InputMove())
				{
					state = State::Run;
					PlayAnimation("Running", true);
				}
			}
			break;
		}
	}

	// トランスフォーム更新処理
	UpdateTransform(elapsedTime);

	// アニメーション更新処理
	UpdateAnimation(elapsedTime);
}

// 描画処理
void AnimationScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// モデル描画
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	modelRenderer->Render(rc, transform, model.get(), ShaderId::Lambert);

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// グリッド描画
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI描画処理
void AnimationScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 460), ImGuiCond_Once);

	if (ImGui::Begin(u8"アニメーション", nullptr, ImGuiWindowFlags_None))
	{
		ImGui::Text(u8"移動操作：WASD");
		ImGui::Text(u8"ジャンプ操作：Space");
		ImGui::Spacing();

		const char* stateName = "";
		switch (state)
		{
		case State::Idle:	stateName = "Idle";	break;
		case State::Run:	stateName = "Run";	break;
		case State::Jump:	stateName = "Jump";	break;
		}
		ImGui::LabelText("State", stateName);

		ImGui::Checkbox("Loop", &animationLoop);
		ImGui::SliderFloat("BlendSeconds", &animationBlendSecondsLength, 0.0f, 1.0f, "%.3f");

		if (model != nullptr)
		{
			const std::vector<ModelResource::Animation>& animations = model->GetResource()->GetAnimations();
			float secondsLength = animationIndex >= 0 ? animations.at(animationIndex).secondsLength : 0;
			int currentFrame = static_cast<int>(animationSeconds * 60.0f);
			int frameLength = static_cast<int>(secondsLength * 60);

			ImGui::SliderFloat("AnimationSeconds", &animationSeconds, 0, secondsLength, "%.3f");

			int index = 0;
			for (const ModelResource::Animation& animation : animations)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

				if (animationIndex == index)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				ImGui::TreeNodeEx(&animation, nodeFlags, animation.name.c_str());

				// クリックでアニメーション再生
				if (ImGui::IsItemClicked())
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						PlayAnimation(index, animationLoop);
					}
				}

				ImGui::TreePop();

				index++;
			}
		}
	}
	ImGui::End();
}

// アニメーション再生
void AnimationScene::PlayAnimation(int index, bool loop)
{
	animationPlaying = true;
	animationLoop = loop;
	animationIndex = index;
	animationSeconds = 0.0f;
}

void AnimationScene::PlayAnimation(const char* name, bool loop)
{
	int index = 0;
	const std::vector<ModelResource::Animation>& animations = model->GetResource()->GetAnimations();
	for (const ModelResource::Animation& animation : animations)
	{
		if (animation.name == name)
		{
			PlayAnimation(index, loop);
			return;
		}
		++index;
	}
}


// アニメーション更新処理
void AnimationScene::UpdateAnimation(float elapsedTime)
{
	if (animationPlaying)
	{
		float blendRate = 1.0f;
		if (animationSeconds < animationBlendSecondsLength)
		{
			animationSeconds += elapsedTime;
			if (animationSeconds >= animationBlendSecondsLength)
			{
				animationSeconds = animationBlendSecondsLength;
			}
			blendRate = animationSeconds / animationBlendSecondsLength;
			blendRate *= blendRate;
		}
		std::vector<Model::Node>& nodes = model->GetNodes();

		const std::vector<ModelResource::Animation>& animations = model->GetResource()->GetAnimations();
		const ModelResource::Animation& animation = animations.at(animationIndex);
		animationSeconds += elapsedTime;

		if (animationSeconds >= animation.secondsLength)
		{
			if (animationLoop)
			{
				animationSeconds -= animation.secondsLength;
			}
			else
			{
				animationPlaying = false;
				animationSeconds = animation.secondsLength;
			}
		}
		
		const std::vector<ModelResource::Keyframe>& keyframes = animation.keyframes;
		int keyCount = static_cast<int>(keyframes.size());
		for (int keyIndex = 0; keyIndex < keyCount - 1; ++keyIndex)
		{
			const ModelResource::Keyframe& keyframe0 = keyframes.at(keyIndex);
			const ModelResource::Keyframe& keyframe1 = keyframes.at(keyIndex+1);
			if (animationSeconds >= keyframe0.seconds && animationSeconds < keyframe1.seconds)
			{
				float rate=(animationSeconds - keyframe0.seconds) / (keyframe1.seconds - keyframe0.seconds);
				int nodeCount = static_cast<int>(nodes.size());
				for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
				{
					const ModelResource::NodeKeyData& key0 = keyframe0.nodeKeys.at(nodeIndex);
					const ModelResource::NodeKeyData& key1 = keyframe1.nodeKeys.at(nodeIndex);
					
					Model::Node& node = nodes[nodeIndex];
					if (blendRate < 1.0f)
					{
						// 現在の姿勢と次のキーフレームとの姿勢の補完
						DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&node.scale);
						DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);
						DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&node.rotate);
						DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);
						DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&node.translate);
						DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

						DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, blendRate);
						DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, blendRate);
						DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, blendRate);
						// 計算結果をボーンに格納
						DirectX::XMStoreFloat3(&node.scale, S);
						DirectX::XMStoreFloat4(&node.rotate, R);
						DirectX::XMStoreFloat3(&node.translate, T);
					}
					else
					{
						// 現在の姿勢と次のキーフレームとの姿勢の補完
						DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&node.scale);
						DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&key1.scale);
						DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&node.rotate);
						DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&key1.rotate);
						DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&node.translate);
						DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&key1.translate);

						DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, rate);
						DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, rate);
						DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, rate);

						// 計算結果をボーンに格納
						DirectX::XMStoreFloat3(&node.scale, S);
						DirectX::XMStoreFloat4(&node.rotate, R);
						DirectX::XMStoreFloat3(&node.translate, T);
					}
				}
				break;
			}
		}
	}
	model->UpdateTransform();
}

// トランスフォーム更新処理
void AnimationScene::UpdateTransform(float elapsedTime)
{
	// 加速処理
	float vecLength = sqrtf(moveVecX * moveVecX + moveVecZ * moveVecZ);
	if (vecLength > 0)
	{
		float vecX = moveVecX / vecLength;
		float vecZ = moveVecZ / vecLength;

		float acceleration = this->acceleration * elapsedTime;
		if (!onGround) acceleration *= airControl;

		velocity.x += vecX * acceleration;
		velocity.z += vecZ * acceleration;

		// 最大速度制限
		float velocityLength = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
		if (velocityLength > moveSpeed)
		{
			velocity.x = (velocity.x / velocityLength) * moveSpeed;
			velocity.z = (velocity.z / velocityLength) * moveSpeed;
		}

		// 進行方向を向くようにする
		{
			// 向いている方向
			float frontX = sinf(angle.y);
			float frontZ = cosf(angle.y);

			// 回転量調整
			float dot = frontX * vecX + frontZ * vecZ;
			float rot = (std::min)(1.0f - dot, turnSpeed * elapsedTime);

			// 左右判定をして回転処理
			float cross = frontX * vecZ - frontZ * vecX;
			if (cross < 0.0f)
			{
				angle.y += rot;
			}
			else
			{
				angle.y -= rot;
			}
		}
	}
	else
	{
		// 減速処理
		float deceleration = this->deceleration * elapsedTime;
		if (!onGround) deceleration *= airControl;

		float velocityLength = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
		if (velocityLength > deceleration)
		{
			velocity.x -= (velocity.x / velocityLength) * deceleration;
			velocity.z -= (velocity.z / velocityLength) * deceleration;
		}
		else
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
	}

	// 重力処理
	velocity.y -= gravity * elapsedTime;

	// 位置更新
	position.x += velocity.x * elapsedTime;
	position.y += velocity.y * elapsedTime;
	position.z += velocity.z * elapsedTime;

	// 地面判定
	if (position.y < 0.0f)
	{
		position.y = 0.0f;
		velocity.y = 0.0f;
		onGround = true;
	}
	else
	{
		onGround = false;
	}


	// 行列計算
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMStoreFloat4x4(&transform, S * R * T);
}

// 移動入力処理
bool AnimationScene::InputMove()
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
	moveVecX = cameraFrontX * axisY + cameraRightX * axisX;
	moveVecZ = cameraFrontZ * axisY + cameraRightZ * axisX;
	float vecLength = sqrtf(moveVecX * moveVecX + moveVecZ * moveVecZ);
	
	return vecLength > 0.0f;
}

// ジャンプ入力処理
bool AnimationScene::InputJump()
{
	if (GetAsyncKeyState(VK_SPACE) & 0x01)
	{
		velocity.y = jumpSpeed;
		return true;
	}
	return false;
}
