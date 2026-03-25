#include "Mesh.h"

bool Mesh::Create(ID3D12Device* device, const void* vertices, uint32_t vertexSize, uint32_t stride, const uint16_t* indices, uint32_t indexCount) {

}

void Mesh::Bind(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->IASetIndexBuffer(&m_indexBufferView);
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) {
	cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}