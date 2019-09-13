#pragma once
#define PY_SSIZE_T_CLEAN
#ifdef _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif

#include <vector>
#include "Types.h"

/*
The Simulation is intended to be a self contained object. It
will be possible to store it and reload its state
*/

// https://docs.python.org/3.7/extending/embedding.html

namespace OpenDLA
{
	class DLASimulation
	{
	private:
		PyObject* m_pModule = nullptr;
		PyObject* m_pUpdateFun = nullptr;
	public:
		// Very simple for now! Should be more intelegent structure
		std::vector<Point> m_points;
	public:
		DLASimulation();
		~DLASimulation();
		bool Initialise();
		void Update();
	};
}