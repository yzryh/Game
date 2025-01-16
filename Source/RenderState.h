#pragma once

#include <wrl.h>
#include <d3d11.h>

// �T���v���X�e�[�g
enum class SamplerState
{
	PointWrap,
	PointClamp,
	LinearWrap,
	LinearClamp,

	EnumCount
};

// �f�v�X�X�e�[�g
enum class DepthState
{
	TestAndWrite,
	TestOnly,
	WriteOnly,
	NoTestNoWrite,

	EnumCount
};

// �u�����h�X�e�[�g
enum class BlendState
{
	Opaque,
	Transparency,
	Additive,
	Subtraction,
	Multiply,

	EnumCount
};

// ���X�^���C�U�X�e�[�g
enum class RasterizerState
{
	SolidCullNone,
	SolidCullBack,
	WireCullNone,
	WireCullBack,

	EnumCount
};

// �����_�[�X�e�[�g
class RenderState
{
public:
	RenderState(ID3D11Device* device);
	~RenderState() = default;

	// �T���v���X�e�[�g�擾
	ID3D11SamplerState* GetSamplerState(SamplerState state) const
	{
		return samplerStates[static_cast<int>(state)].Get();
	}

	// �f�v�X�X�e�[�g�擾
	ID3D11DepthStencilState* GetDepthStencilState(DepthState state) const
	{
		return depthStencilStates[static_cast<int>(state)].Get();
	}

	// �u�����h�X�e�[�g�擾
	ID3D11BlendState* GetBlendState(BlendState state) const
	{
		return blendStates[static_cast<int>(state)].Get();
	}

	// ���X�^���C�U�[�X�e�[�g�擾
	ID3D11RasterizerState* GetRasterizerState(RasterizerState state) const
	{
		return rasterizerStates[static_cast<int>(state)].Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		samplerStates[static_cast<int>(SamplerState::EnumCount)];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[static_cast<int>(DepthState::EnumCount)];
	Microsoft::WRL::ComPtr<ID3D11BlendState>		blendStates[static_cast<int>(BlendState::EnumCount)];
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	rasterizerStates[static_cast<int>(RasterizerState::EnumCount)];
};
