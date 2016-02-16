#include "../include/rendering/pipeline.h"

using namespace Basilisk;


DXGI_FORMAT translate(InputFormat format, unsigned int count)
{
	return DXGI_FORMAT_B8G8R8A8_TYPELESS;
}

D3D12Pipeline::D3D12Pipeline(D3D12Device *device)
{
	signature = nullptr;
	error = nullptr;
}

inline ID3D12PipelineState *D3D12Pipeline::getPipelineState()
{
	return m_pso;
}

template<class T> void safeRelease(T *obj)
{
	if (obj != nullptr)
	{
		obj->Release();
		obj = nullptr;
	}
}

void D3D12Pipeline::release()
{
	safeRelease(m_pso);
}