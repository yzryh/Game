#include <imgui.h>
#include <algorithm>
#include "Graphics.h"
#include "Misc.h"
#include "GpuResourceUtils.h"
#include "Scene/UIAnimScene.h"

// コンストラクタ
UIAnimScene::UIAnimScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	HRESULT hr;

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(Vertex) * 24;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// 頂点バッファオブジェクトの生成
		hr = device->CreateBuffer(&buffer_desc, nullptr, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// 頂点シェーダー
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		hr = GpuResourceUtils::LoadVertexShader(
			device,
			"Data/Shader/SpriteVS.cso",
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			inputLayout.GetAddressOf(),
			vertexShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	}

	// ピクセルシェーダー
	{
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// テクスチャ
	{
		hr = GpuResourceUtils::LoadTexture(device, "Data/Sprite/jammo.png", shaderResourceView.GetAddressOf(), &textureDesc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}

// 更新処理
void UIAnimScene::Update(float elapsedTime)
{
	auto updateAmount = [&](float& amount)
	{
		amount += 0.5f * elapsedTime;
		if (amount > 1.0f)
		{
			amount = 0.0f;
		}
	};
	updateAmount(easingAmount);
	updateAmount(horizontalFillAmount);
	updateAmount(verticalFillAmount);
	updateAmount(radialFillAmount);
}

// 描画処理
void UIAnimScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();

	// シェーダー設定
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetInputLayout(inputLayout.Get());

	// 画面サイズ＆テクスチャサイズ取得
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();
	float textureWidth = static_cast<float>(textureDesc.Width);
	float textureHeight = static_cast<float>(textureDesc.Height);

	// 描画
	DrawEasing(dc, 600, 50, 300, 300, 0, 0, textureWidth, textureHeight, easingAmount, screenWidth, screenHeight);
	DrawFillHorizontal(dc, 100, 400, 300, 300, 0, 0, textureWidth, textureHeight, horizontalFillAmount, screenWidth, screenHeight);
	DrawFillVertical(dc, 410, 400, 300, 300, 0, 0, textureWidth, textureHeight, verticalFillAmount, screenWidth, screenHeight);
	DrawFillRadial(dc, 720, 400, 300, 300, 0, 0, textureWidth, textureHeight, radialFillAmount, screenWidth, screenHeight);
}

// GUI描画処理
void UIAnimScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 180), ImGuiCond_Once);

	if (ImGui::Begin(u8"UI", nullptr, ImGuiWindowFlags_NoNavInputs))
	{
		ImGui::SliderFloat("EasingAmount", &easingAmount, 0.0f, 1.0f);
		ImGui::SliderFloat("HorizontalFillAmount", &horizontalFillAmount, 0.0f, 1.0f);
		ImGui::SliderFloat("VerticalFillAmount", &verticalFillAmount, 0.0f, 1.0f);
		ImGui::SliderFloat("RadialFillAmount", &radialFillAmount, 0.0f, 1.0f);
	}
	ImGui::End();
}

// スプライト描画
void UIAnimScene::DrawSprite(
	ID3D11DeviceContext* dc,
	float dx, float dy, float dw, float dh,
	float sx, float sy, float sw, float sh,
	float screenWidth,
	float screenHeight) const
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	Vertex* v = reinterpret_cast<Vertex*>(mappedSubresource.pData);

	v[0].position.x = dx;
	v[0].position.y = dy;
	v[0].texcoord.x = sx;
	v[0].texcoord.y = sy;

	v[1].position.x = dx+dw;
	v[1].position.y = dy;
	v[1].texcoord.x = sx+sw;
	v[1].texcoord.y = sy;

	v[2].position.x = dx;
	v[2].position.y = dy+dh;
	v[2].texcoord.x = sx;
	v[2].texcoord.y = sy+sh;

	v[3].position.x = dx+dw;
	v[3].position.y = dy+dh;
	v[3].texcoord.x = sx+sw;
	v[3].texcoord.y = sy+sh;

	for (int i = 0; i < 4; i++)
	{
		v[i].color = { 1,1,1,1 };
		v[i].position.x = 2.0f * v[i].position.x / screenWidth - 1.0f;
		v[i].position.y = 1.0f - 2.0f * v[i].position.y / screenHeight;
		v[i].position.z = 0;
		v[i].texcoord.x = v[i].texcoord.x / textureDesc.Width;
		v[i].texcoord.y = v[i].texcoord.y / textureDesc.Height;
	}
	dc->Unmap(vertexBuffer.Get(), 0);
	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->Draw(4, 0);
	
}

// 水平塗りつぶし描画
void UIAnimScene::DrawFillHorizontal(
	ID3D11DeviceContext* dc,
	float dx, float dy, float dw, float dh,	// 表示座標＆サイズ
	float sx, float sy, float sw, float sh,	// テクスチャ座標＆サイズ
	float amount,							// 描画率(0.0～1.0)
	float screenWidth,
	float screenHeight) const
{
	dw *= amount;
	sw *= amount;
	DrawSprite(dc, dx, dy,dw,dh,sx, sy, sw, sh,screenWidth,screenHeight);
}

// 垂直塗りつぶし描画
void UIAnimScene::DrawFillVertical(
	ID3D11DeviceContext* dc,
	float dx, float dy, float dw, float dh,
	float sx, float sy, float sw, float sh,
	float amount,
	float screenWidth,
	float screenHeight) const
{
	dh *= amount;
	sh *= amount;
	DrawSprite(dc, dx, dy, dw, dh, sx, sy, sw, sh, screenWidth, screenHeight);
}

// イージング描画
void UIAnimScene::DrawEasing(
	ID3D11DeviceContext* dc,
	float dx, float dy, float dw, float dh,
	float sx, float sy, float sw, float sh,
	float amount,
	float screenWidth,
	float screenHeight) const
{
	float offset = 150;
	dw *= 0.5f;
	sw *= 0.5f;
	auto easeOutBounce = [](float x)->float
		{
			const float n1 = 7.5625f;
			const float d1 = 2.75f;

			if (x < 1 / d1) {
				return n1 * x * x;
			}
			else if (x < 2 / d1) {
				return n1 * (x -= 1.5f / d1) * x + 0.75f;
			}
			else if (x < 2.5 / d1) {
				return n1 * (x -= 2.25f / d1) * x + 0.9375f;
			}
			else {
				return n1 * (x -= 2.625f / d1) * x + 0.984375f;
			}
			return x;
		};
	auto lerp = [](float v1, float v2, float t)->float
		{
			return (1.0f - t) * v1 + t * v2;
		};
	offset = lerp(offset, 0, easeOutBounce(amount));
		DrawSprite(dc, dx - offset, dy, dw,dh, sx, sy, sw, sh, screenWidth, screenHeight);
	DrawSprite(dc, dx + dw + offset, dy, dw, dh, sx + sw, sy, sw, sh, screenWidth, screenHeight);
}

// 放射塗りつぶし描画
void UIAnimScene::DrawFillRadial(
	ID3D11DeviceContext* dc,
	float dx, float dy, float dw, float dh,
	float sx, float sy, float sw, float sh,
	float amount,
	float screenWidth,
	float screenHeight) const
{
	//  +----+----+
	//  |0   |1   |2
	//  |    |    |
	//  +----+----+
	//  |3   |4   |5
	//  |    |    |
	//  +----+----+
	//   6    7    8
	Vertex p[9];
	p[0].position.x = p[3].position.x = p[6].position.x = dx;
	p[1].position.x = p[4].position.x = p[7].position.x = dx + dw * 0.5f;
	p[2].position.x = p[5].position.x = p[8].position.x = dx + dw;

	p[0].position.y = p[1].position.y = p[2].position.y = dy;
	p[3].position.y = p[4].position.y = p[5].position.y = dy + dh * 0.5f;
	p[6].position.y = p[7].position.y = p[8].position.y = dy + dh;

	p[0].texcoord.x = p[3].texcoord.x = p[6].texcoord.x = sx;
	p[1].texcoord.x = p[4].texcoord.x = p[7].texcoord.x = sx + sw * 0.5f;
	p[2].texcoord.x = p[5].texcoord.x = p[8].texcoord.x = sx + sw;

	p[0].texcoord.y = p[1].texcoord.y = p[2].texcoord.y = sy;
	p[3].texcoord.y = p[4].texcoord.y = p[5].texcoord.y = sy + sh * 0.5f;
	p[6].texcoord.y = p[7].texcoord.y = p[8].texcoord.y = sy + sh;

	// 頂点編集開始
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Vertex* v = reinterpret_cast<Vertex*>(mappedSubresource.pData);

	// 進行度(amount)に応じてスプライトが円を描くように表示されるようにする
	{
		// +----+----+
		// |＼7 | 0／|
		// | 6＼|／1 |
		// +----+----+
		// | 5／|＼2 |
		// |／4 | 3＼|
		// +----+----+
		//
		float t0 = (std::clamp)(amount - 0.125f * 0, 0.0f, 0.125f) / 0.125f;
		float t1= (std::clamp)(amount - 0.125f * 1, 0.0f, 0.125f) / 0.125f;
		float t2= (std::clamp)(amount - 0.125f * 2, 0.0f, 0.125f) / 0.125f;
		float t3= (std::clamp)(amount - 0.125f * 3, 0.0f, 0.125f) / 0.125f;
		float t4= (std::clamp)(amount - 0.125f * 4, 0.0f, 0.125f) / 0.125f;
		float t5= (std::clamp)(amount - 0.125f * 5, 0.0f, 0.125f) / 0.125f;
		float t6= (std::clamp)(amount - 0.125f * 6, 0.0f, 0.125f) / 0.125f;
		float t7= (std::clamp)(amount - 0.125f * 7, 0.0f, 0.125f) / 0.125f;
		auto lerp = [&](const Vertex& v1, const Vertex& v2, float t)->Vertex
			{
				DirectX::XMVECTOR P1 = DirectX::XMLoadFloat3(&v1.position);
				DirectX::XMVECTOR P2 = DirectX::XMLoadFloat3(&v2.position);
				DirectX::XMVECTOR P3 = DirectX::XMVectorLerp(P1, P2, t);
				DirectX::XMVECTOR T1 = DirectX::XMLoadFloat2(&v1.texcoord);
				DirectX::XMVECTOR T2 = DirectX::XMLoadFloat2(&v2.texcoord);
				DirectX::XMVECTOR T3 = DirectX::XMVectorLerp(T1, T2, t);

				Vertex out;
				DirectX::XMStoreFloat3(&out.position, P3);
				DirectX::XMStoreFloat2(&out.texcoord, T3);
				return out;
			};
		v[0] = p[1];
		v[1] = lerp(p[1], p[2], t0);
		v[2] = p[4];

		v[3] = p[2];
		v[4] = lerp(p[2], p[5], t1);
		v[5] = p[4];

		v[6] = p[5];
		v[7] = lerp(p[5], p[8], t2);
		v[8] = p[4];

		v[9] = p[8];
		v[10] = lerp(p[8], p[7], t3);
		v[11] = p[4];
			
		v[12] = p[7];
		v[13] = lerp(p[7],p[6],t4);
		v[14] = p[4];

		v[15] = p[6];
		v[16] = lerp(p[6], p[3], t5);
		v[17] = p[4];

		v[18] = p[3];
		v[19] = lerp(p[3], p[0], t6);
		v[20] = p[4];

		v[21] = p[0];
		v[22] = lerp(p[0], p[1], t7);
		v[23] = p[4];
	}

	// NDC座標変換＆UV座標変換
	for (int i = 0; i < 24; ++i)
	{
		v[i].color = { 1, 1, 1, 1 };
		v[i].position.x = 2.0f * v[i].position.x / screenWidth - 1.0f;
		v[i].position.y = 1.0f - 2.0f * v[i].position.y / screenHeight;
		v[i].position.z = 0.0f;
		v[i].texcoord.x = v[i].texcoord.x / textureDesc.Width;
		v[i].texcoord.y = v[i].texcoord.y / textureDesc.Height;
	}

	// 頂点編集終了
	dc->Unmap(vertexBuffer.Get(), 0);

	// シェーダーリソースビュー設定
	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	// プリミティブトポロジー設定
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画
	dc->Draw(24, 0);

}
