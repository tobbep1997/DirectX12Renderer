#include "DirectX12EnginePCH.h"
#include "X12Timer.h"
#include <fstream>

X12Timer::X12Timer()
{
}


X12Timer::~X12Timer()
{
}

HRESULT X12Timer::CreateTimer(const UINT& numberOfIndexes, ID3D12Device * device, const D3D12_QUERY_HEAP_TYPE& heapType)
{

	m_pDevice = device ? device : RenderingManager::GetInstance()->GetMainAdapter()->GetDevice();
	HRESULT hr = 0;

	m_timerCount = numberOfIndexes;

	D3D12_QUERY_HEAP_DESC queryHeapDesc;
	queryHeapDesc.Type = heapType;
	queryHeapDesc.NodeMask = 0;
	queryHeapDesc.Count = m_timerCount * 2;

	if (SUCCEEDED(hr = m_pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_queryHeap))))
	{
		D3D12_RESOURCE_DESC resourceDesc;
		ZeroMemory(&resourceDesc, sizeof(resourceDesc));
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = sizeof(TimeStamp) * m_timerCount;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = D3D12_HEAP_TYPE_READBACK;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 1;
		heapProp.VisibleNodeMask = 1;

		if (SUCCEEDED(hr = m_pDevice->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_queryResourceCPU))
		))
		{
			m_queryResourceCPU->SetName(L"queryResourceCPU_");
		}

		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		if (SUCCEEDED(hr = m_pDevice->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			nullptr,
			IID_PPV_ARGS(&m_queryResourceGPU))
		))
		{
			m_queryResourceGPU->SetName(L"queryResourceGPU_");
		}
	}

	return hr;
}

void X12Timer::Release()
{
	SAFE_RELEASE(m_queryHeap);
	SAFE_RELEASE(m_queryResourceCPU);
	SAFE_RELEASE(m_queryResourceGPU);
}

void X12Timer::SetCommandQueue(ID3D12CommandQueue* commandQueue)
{
	this->m_pCommandQueue = commandQueue;
}

ID3D12CommandQueue* X12Timer::GetCommandQueue() const
{
	return this->m_pCommandQueue;
}

void X12Timer::Start(ID3D12GraphicsCommandList* commandList)
{
	if (m_active)
		return;
	_start(commandList, m_counter);
}

void X12Timer::Stop(ID3D12GraphicsCommandList* commandList)
{
	if (!m_active)
		return;
	_stop(commandList, m_counter);
}

void X12Timer::ResolveQueryToCpu(ID3D12GraphicsCommandList* commandList) const
{
	if (m_counter >= m_timerCount)
		return;

	commandList->ResolveQueryData(
		m_queryHeap,
		D3D12_QUERY_TYPE_TIMESTAMP,
		m_counter * 2,
		2,
		m_queryResourceCPU,
		sizeof(TimeStamp) * m_counter
	);
}

void X12Timer::CountTimer()
{
	m_counter++;
}

X12Timer::TimeStamp X12Timer::GetTimeStamp(const UINT& index) const
{
	TimeStamp p{};

	TimeStamp* mapMem = nullptr;
	const D3D12_RANGE readRange{ sizeof(p) * index, sizeof(p) * (index + 1) };
	const D3D12_RANGE writeRange{ 0, 0 };
	if (SUCCEEDED(m_queryResourceCPU->Map(0, &readRange, (void**)&mapMem)))
	{
		mapMem += index;
		p = *mapMem;
		m_queryResourceCPU->Unmap(0, &writeRange);
	}

	return p;
}

const UINT& X12Timer::GetCount() const
{
	return m_counter;
}

void X12Timer::PrintToFile(const char* path) const
{
	std::ofstream out(path);

	if (out)
	{
		for (UINT i = 0; i < m_timerCount; i++)
		{
			const TimeStamp stamp = GetTimeStamp(i);

			if (m_pCommandQueue)
			{
				UINT64 queueFreq;
				m_pCommandQueue->GetTimestampFrequency(&queueFreq);
				const double timestampToMs = (1.0 / queueFreq) * 1000.0;

				out << "Start\t" << stamp.Start << "\tEnd:\t" << stamp.Stop << "\tTot:\t" << stamp.Stop - stamp.Start << "\tdt:\t" << (stamp.Stop - stamp.Start) * timestampToMs << " ms" <<  std::endl;


			}
			else
				out << "Start\t" << stamp.Start << "\tEnd:\t" << stamp.Stop << "\tTot:\t" << stamp.Stop - stamp.Start << std::endl;

		}
		out.close();
	}	
}

void X12Timer::ExportToMagnusTimelineCreator(const char* path) const
{
	if (!m_pCommandQueue)
		throw "No CommandQueue";


	std::ofstream out(path);
	if (out)
	{
		for (UINT i = 0; i < m_timerCount; i++)
		{
			const TimeStamp stamp = GetTimeStamp(i);

			UINT64 queueFreq;
			m_pCommandQueue->GetTimestampFrequency(&queueFreq);
			const double timestampToMs = (1.0 / queueFreq) * 1000.0;

			out << stamp.Start << "\t" << stamp.Stop << "\t" << (stamp.Stop - stamp.Start) * timestampToMs << std::endl;
		}
		out.close();
	}
}

void X12Timer::_start(ID3D12GraphicsCommandList* commandList, const UINT& timeStampIndex)
{
	if (timeStampIndex >= m_timerCount)
		return;
	m_active = true;
	commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampIndex * 2);
}

void X12Timer::_stop(ID3D12GraphicsCommandList* commandList, const UINT& timeStampIndex)
{
	if (timeStampIndex >= m_timerCount)
		return;
	m_active = false;
	commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampIndex * 2 + 1);
}
