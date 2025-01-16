#include "Misc.h"
#include "GpuResourceUtils.h"
#include "LambertShader.h"

LambertShader::LambertShader(ID3D11Device* device)
{
	// ���_�V�F�[�_�[
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/LambertVS.cso",
		ModelResource::InputElementDescs.data(),
		static_cast<UINT>(ModelResource::InputElementDescs.size()),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	// �s�N�Z���V�F�[�_�[
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/LambertPS.cso",
		pixelShader.GetAddressOf());

	// �}�e���A���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbMaterial),
		constantBuffer.GetAddressOf());
}

// �J�n����
void LambertShader::Begin(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// �V�F�[�_�[�ݒ�
	dc->IASetInputLayout(inputLayout.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	// �萔�o�b�t�@�ݒ�
	ID3D11Buffer* cbs[] =
	{
		constantBuffer.Get(), 
	};
	dc->PSSetConstantBuffers(1, _countof(cbs), cbs);
}

// �X�V����
void LambertShader::Update(const RenderContext& rc, const ModelResource::Material& material)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// �}�e���A���p�萔�o�b�t�@�X�V
	CbMaterial cbMaterial{};
	cbMaterial.materialColor = material.color;
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbMaterial, 0, 0);

	// �V�F�[�_�[���\�[�X�r���[�ݒ�
	ID3D11ShaderResourceView* srvs[] =
	{
		material.shaderResourceView.Get(),
	};
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}

// �`��I��
void LambertShader::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// �V�F�[�_�[�ݒ����
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->IASetInputLayout(nullptr);

	// �萔�o�b�t�@�ݒ����
	ID3D11Buffer* cbs[] = { nullptr };
	dc->PSSetConstantBuffers(1, _countof(cbs), cbs);

	// �V�F�[�_�[���\�[�X�r���[�ݒ����
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}
