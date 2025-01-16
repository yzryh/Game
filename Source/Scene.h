#pragma once

// �V�[�����
class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	// �X�V����
	virtual void Update(float elapsedTime) {}

	// �`�揈��
	virtual void Render(float elapsedTime) {}

	// GUI�`�揈��
	virtual void DrawGUI() {}
};
