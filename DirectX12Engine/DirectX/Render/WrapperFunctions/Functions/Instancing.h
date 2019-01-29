#pragma once
#include <vector>
#include "DirectX/Objects/Drawable.h"
#include "DirectX/Objects/Mesh/StaticMesh.h"
#include "DirectX12Engine.h"

class Texture;
class Transform;
class StaticMesh;

namespace Instancing
{
	struct InstanceBuffer
	{
		InstanceBuffer() = default;
		InstanceBuffer(const DirectX::XMFLOAT4X4 & worldMatrix)
		{
			WorldMatrix = worldMatrix;
		}
		DirectX::XMFLOAT4X4 WorldMatrix;
	};

	struct InstanceGroup
	{
		const StaticMesh * StaticMesh;

		const Texture * Albedo;
		const Texture * Normal;
		const Texture * Metallic;
		const Texture * Displacement;

		InstanceBuffer Transforms[1024];

		InstanceGroup(Drawable * drawable)
		{
			StaticMesh = drawable->GetMesh();
			Albedo = drawable->GetTexture();
			Normal = drawable->GetNormal();
			Metallic = drawable->GetMetallic();
			Displacement = drawable->GetDisplacement();
			
			currentIndex = 0;
			Add(drawable->GetWorldMatrix());
		}
		void MapTextures(const UINT& rootParameterIndex, ID3D12GraphicsCommandList * commandList)const
		{
			const Texture * arr[5] = {Albedo, Normal, Metallic, Displacement, Normal};
			
			for (UINT i = 0; i < 5; i++)
			{	
				arr[i]->MapTexture(nullptr, rootParameterIndex + i, commandList);
			}
		}
		void Add(const DirectX::XMFLOAT4X4 & worldMatrix)
		{
			Transforms[currentIndex++] = InstanceBuffer(worldMatrix);
		}
		const UINT & GetSize() const
		{
			return currentIndex;
		}
		const UINT & GetMaxSize() const
		{
			return maxSize;
		}
	private:
		UINT currentIndex = 0;
		UINT maxSize = 1024;
		
	};

	inline bool MatchGroup(InstanceGroup group, Drawable * drawable)
	{
		return drawable->GetMesh() == group.StaticMesh &&
			drawable->GetTexture() == group.Albedo &&
			drawable->GetNormal() == group.Normal &&
			drawable->GetMetallic() == group.Metallic &&
			drawable->GetDisplacement() == group.Displacement;
	}

	inline void AddInstance(std::vector<InstanceGroup> * instanceGroups, Drawable * drawable)
	{
		InstanceGroup * currentInstanceGroup = nullptr;
		for (unsigned int group = 0; group < instanceGroups->size(); group++)
		{
			if (MatchGroup(instanceGroups->at(group), drawable))
			{
				currentInstanceGroup = &instanceGroups->at(group);	
			}
		}

		if (currentInstanceGroup)
		{			
			currentInstanceGroup->Add(drawable->GetWorldMatrix());
		}
		else
		{
			const InstanceGroup newInstanceGroup = InstanceGroup(drawable);
			instanceGroups->push_back(newInstanceGroup);
		}
	}

	inline void ClearInstanceGroup(std::vector<InstanceGroup>* instanceGroups)
	{
		instanceGroups->clear();
	}

	inline UINT64 UpdateInstanceGroup(
		ID3D12GraphicsCommandList * commandList,
		D3D12_VERTEX_BUFFER_VIEW & instanceBufferView,
		ID3D12Resource * dstResource, 
		ID3D12Resource * intermediate, 
		std::vector<InstanceGroup> * groups, 
		UINT structSize = sizeof(InstanceBuffer),
		const UINT64 & offset = 0)
	{
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dstResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));
		UINT64 dataSize = 0;
		UINT current = 0;
		InstanceBuffer arr[4096];
		for (size_t i = 0; i < groups->size(); i++)
		{
			dataSize += static_cast<UINT64>(structSize * groups->at(i).GetSize());
			for (size_t j = 0; j < groups->at(i).GetSize(); j++)
			{
				arr[current++] = groups->at(i).Transforms[j];
			}
		}

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedSubresourceFootprint;
		UINT pNumRows;
		UINT64 pBytesPerRow;
		UINT64 pTotalSize;

		ID3D12Device * device = nullptr;
		dstResource->GetDevice(IID_PPV_ARGS(&device));

		D3D12_RESOURCE_DESC dstResourceDesc = dstResource->GetDesc();
		device->GetCopyableFootprints(
			&dstResourceDesc,
			0,
			1,
			offset,
			&placedSubresourceFootprint,
			&pNumRows,
			&pBytesPerRow,
			&pTotalSize);

		device->Release();		

		D3D12_SUBRESOURCE_DATA instanceData = {};
		instanceData.pData = arr;
		instanceData.RowPitch = dataSize;
		instanceData.SlicePitch = dataSize;

		const UINT64 retValue = UpdateSubresources(
			commandList, 
			dstResource, 
			intermediate, 
			0, 1, pTotalSize,
			&placedSubresourceFootprint,
			&pNumRows,
			&pTotalSize,
			&instanceData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dstResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		if (retValue)
			return dataSize;
		return retValue;
	}

}
