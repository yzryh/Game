#include "Character.h"
namespace Characters
{
    Characters::Character::Character(const std::string modelPath, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, Character* enemy)
        :
        state(CharacterState::None),
        enemy(enemy)
    {
        this->position = position;
        this->scale = scale;
        this->model = std::make_unique<Model>(modelPath.c_str());
       
    }

    void Characters::Character::Update(float elapsedTime)
    {

        switch (state)
        {
        case CharacterState::None:
            InputHandler(elapsedTime);
            break;
        case CharacterState::Attack:
            Attack(elapsedTime);
            break;
        case CharacterState::Guard:
            Guard(elapsedTime);
            break;
        case CharacterState::Stan:
            Stan(elapsedTime);
            break;
        case CharacterState::jump:
            jump(elapsedTime);
            break;
        }
    }

    void Characters::Character::Render(ID3D11DeviceContext* dc, const RenderState* renderState, ModelRenderer* modelRenderer, const Camera* camera)
    {
        RenderContext rc;
        rc.deviceContext = dc;
        rc.renderState = renderState;
        rc.camera = camera;
        modelRenderer->Render(rc, transform, model.get(), ShaderId::Lambert);
    }

    void Characters::Character::DrawDebugPrimitive(ShapeRenderer* shapeRenderer)
    {
    }

    void Characters::Character::InputHandler(float elapsedTime)
    {

    }

    DirectX::XMFLOAT3 Characters::Character::GetMoveVec()
    {
        return DirectX::XMFLOAT3();
    }

    void Characters::Character::Move(float x, float z, float elapsedTime)
    {
    }

    void Characters::Character::Turn(float x, float z, float elapsedTime)
    {
    }

    void Characters::Character::Attack(float elapsedTime)
    {
    }

    void Characters::Character::Guard(float elapsedTime)
    {
    }

    void Characters::Character::Stan(float elapsedTime)
    {
    }

    void Character::jump(float elpsedTime)
    {
        if (gamePad.GetButtonDown() & GamePad::BTN_UP)
        {
            velocity.y = jumpSpeed;
        }
    }

    void Characters::Character::GetAction()
    {
    }

    void Characters::Character::lowHp(DirectX::XMFLOAT3 position)
    {
    }

    void Characters::Character::CollisionPlayerVsEnemies()
    {
    }

    void Characters::Character::CollisionAttack(DirectX::XMFLOAT3 attackPosition)
    {
    }

    void Characters::Character::CollisionPush(DirectX::XMFLOAT3 pushPosition)
    {
    }

    void Characters::Character::DrawDebugGUI()
    {
    }

    void Characters::Character::DrawAttackPrimitive(DirectX::XMFLOAT3 attackPosition)
    {
    }

    void Characters::Character::death()
    {

    }

    void Characters::Character::DrawPushPrimitive(DirectX::XMFLOAT3 pushPosition)
    {

    }
}