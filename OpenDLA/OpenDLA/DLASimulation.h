#pragma once

#include <vector>
#include "Types.h"

/*
The Simulation is intended to be a self contained object. It
will be possible to store it and reload its state
*/

namespace OpenDLA
{
	class DLASimulation
	{
	public:
		// Very simple for now! Should be more intelegent structure
		std::vector<Point> m_points;
	public:
		DLASimulation();
		void Update();
	};
}