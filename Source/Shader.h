#pragma once

#include "RenderContext.h"
#include "Model.h"

class Shader
{
public:
	Shader() {}
	virtual ~Shader() {}

	// �J�n����
	virtual void Begin(const RenderContext& rc) = 0;

	// �X�V����
	virtual void Update(const RenderContext& rc, const ModelResource::Material& material) = 0;

	// �I������
	virtual void End(const RenderContext& rc) = 0;
};
