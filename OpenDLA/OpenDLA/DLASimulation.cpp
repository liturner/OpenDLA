#include "DLASimulation.h"

OpenDLA::DLASimulation::DLASimulation()
{
	Point a;
	a.pos = DirectX::XMFLOAT3(0, 0, 0);
	a.color = DirectX::XMFLOAT3(1, 0, 0);

	m_points.push_back(a);
}

void OpenDLA::DLASimulation::Update()
{
	m_points[0].pos.x += 1;
}

