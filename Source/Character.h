#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <Model.h>
#include "ModelRenderer.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "RenderState.h"
#include "Input/GamePad.h"
#include "ShapeRenderer.h"
//#include "Effect.h"
#include<vector>
//#include"Audio.h"
//#include"AudioSource.h"
namespace Characters
{
	enum  class CharacterState
	{
		None,
		Stan,
		Attack,
		Guard,
		jump,

	};

	// ‚«‚á‚ç‚ÌŠî’êƒNƒ‰ƒX
	class Character
	{
	public:
		Character() = delete;
		Character(const Character&) = delete;
		Character& operator=(const Character&) = delete;
		Character(const std::string modelPath, const DirectX::XMFLOAT3& position = { 0.0f, 0.0f, 0.0f }, const DirectX::XMFLOAT3& scale = { 1, 1, 1 }, Character* enemy = nullptr);
		virtual ~Character() = default;
		virtual void Update(float elapsedTime);
		virtual void Render(ID3D11DeviceContext* dc, const RenderState* renderState, ModelRenderer* modelRenderer, const Camera* camera);
		void DrawDebugPrimitive(ShapeRenderer* shapeRenderer);

		void SetEnemy(Character* enem) { enemy = enem; }
		int GetHealth() { return health; }

		virtual const std::string GetName() = 0;
		const DirectX::XMFLOAT3 GetPos()const { return position; }

	protected:

		virtual void Attack(float elapsedTime);
		virtual void Guard(float elapsedTime);
		virtual void Stan(float elapsedTime);

		const DirectX::XMFLOAT4X4& UpdateTransform()
		{
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			//DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
			DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(quaternion);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
			DirectX::XMMATRIX WorldTransform = S * R * T;
			DirectX::XMStoreFloat4x4(&transform, WorldTransform);
			model->UpdateTransform();

			return GetTransform();
		}
		const DirectX::XMFLOAT4X4& GetTransform()
		{
			return transform;
		}
	private:
		void InputHandler(float elapsedTime);
		DirectX::XMFLOAT3 GetMoveVec();
		void Move(float x, float z, float elapsedTime);
		void Turn(float x, float z, float elapsedTime);

		void Attack(float elapsedTime);
		void Guard(float elapsedTime);
		void Stan(float elapsedTime);
		void jump(float elpsedTime);
		void GetAction();

		void lowHp(DirectX::XMFLOAT3 position);
		void CollisionPlayerVsEnemies();
		void CollisionAttack(DirectX::XMFLOAT3 attackPosition);
		void CollisionPush(DirectX::XMFLOAT3 pushPosition);
		void DrawDebugGUI();
		void DrawAttackPrimitive(DirectX::XMFLOAT3 attackPosition);
		void death();
		void Character::DrawPushPrimitive(DirectX::XMFLOAT3 pushPosition);

	protected:

		bool					onGround = false;
		DirectX::XMFLOAT3		velocity = { 0, 0, 0 };
		DirectX::XMFLOAT3		position = { 0, 0, 0 };
		DirectX::XMFLOAT3		angle = { 0, 0, 0 };
		DirectX::XMFLOAT3		scale = { 1, 1, 1 };
		DirectX::XMFLOAT4X4		transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		DirectX::XMFLOAT3       front = { 0, 0, 1 };
		
		float					radius = 1.5f;
		float					height = 10.0f;
		float					speed = 0.7f;
		float					jumpSpeed = 5.0f;
		int						timer = 0;
		bool					isDeath = false;
		std::unique_ptr<Model>	model;
		int health = 5;
		bool onHit = false;
		float noHitTimer = 0.0f;
		bool hitFlag = false;
		int endTimer = 0;

		CharacterState state;
		GamePad gamePad;
		Character* enemy;
		DirectX::XMVECTOR quaternion;
	};

}
