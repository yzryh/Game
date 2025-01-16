#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "ModelResource.h"

// ���f��
class Model
{
public:
	Model(const char* filename);
	~Model() {}

	struct Node
	{
		const char*			name;
		Node*				parent;
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;
		DirectX::XMFLOAT4X4	localTransform;
		DirectX::XMFLOAT4X4	globalTransform;

		std::vector<Node*>	children;
	};

	// �s��v�Z
	void UpdateTransform();

	// �m�[�h���X�g�擾
	const std::vector<Node>& GetNodes() const { return nodes; }
	std::vector<Node>& GetNodes() { return nodes; }

	// ���\�[�X�擾
	const ModelResource* GetResource() const { return resource.get(); }

private:
	std::shared_ptr<ModelResource>	resource;
	std::vector<Node>				nodes;
};
