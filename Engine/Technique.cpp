#include "pch.h"
#include "Technique.h"

void Technique::Draw(UINT pass, UINT vertexCount, UINT startVertexLocation)
{
	passes[pass].Draw(vertexCount, startVertexLocation);
}

void Technique::DrawIndexed(UINT pass, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	passes[pass].DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void Technique::DrawInstanced(UINT pass, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	passes[pass].DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Technique::DrawIndexedInstanced(UINT pass, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	passes[pass].DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void Technique::Dispatch(UINT pass, UINT x, UINT y, UINT z)
{
	passes[pass].Dispatch(x, y, z);
}