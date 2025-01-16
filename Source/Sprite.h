#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

// �X�v���C�g
class Sprite
{
public:
	Sprite(ID3D11Device* device);
	Sprite(ID3D11Device* device, const char* filename);

	// ���_�f�[�^
	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};

	// �`����s
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					// ����ʒu
		float dz,							// ���s
		float dw, float dh,					// ���A����
		float sx, float sy,					// �摜�؂蔲���ʒu
		float sw, float sh,					// �摜�؂蔲���T�C�Y
		float angle,						// �p�x
		float r, float g, float b, float a	// �F
	) const;

	// �`����s�i�e�N�X�`���؂蔲���w��Ȃ��j
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					// ����ʒu
		float dz,							// ���s
		float dw, float dh,					// ���A����
		float angle,						// �p�x
		float r, float g, float b, float a	// �F
	) const;

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;
};
