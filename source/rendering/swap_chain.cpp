/**
\file   swap_chain.cpp
\author Andrew Baxter
\date   February 27, 2016

Implements the `SwapChain` template for Direct3D 12 and Vulkan

*/

#include "rendering/swap_chain.h"

using namespace Basilisk;

D3D12SwapChain::D3D12SwapChain() : m_swapChain(nullptr), m_renderTargetViewHeap(nullptr), m_backBufferRenderTarget(nullptr), m_bufferIndex(0) {
}

VulkanSwapChain::VulkanSwapChain() : m_swapChain(VK_NULL_HANDLE), m_bufferIndex(0), m_prepared(false)
{

}