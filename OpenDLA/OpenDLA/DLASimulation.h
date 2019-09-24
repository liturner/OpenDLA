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
		
	public:
		// Very simple for now! Should be more intelegent structure
		std::vector<Point> m_points;
		std::vector<unsigned int> m_walkers;

	public:
		DLASimulation();
		~DLASimulation();
		bool Initialise();
		void Update();
		void Destroy();

	// Internal Checks
	private:
		bool Collides(const Point& _point);

	// The API which python ma
	private:
		GetPoints // Returns All Points (possibly with query parameters or filter)
		PyObject* GetPoint(PyObject* self, PyObject* args);
		GetNumPoints

		SetPoint // Change an existing point
		Add Point // Append a new point

		PyObject* AddPoint(PyObject* self, PyObject* args);
		
		PyObject* SetWalker(PyObject* self, PyObject* args);
		PyObject* GetWalker(PyObject* self, PyObject* args);

	// Python Function Calls. These map to calls in the Python files
	// Mentality is a 1:1 mapping, so what is returned in python will be returned here
	private:

		/// <summary>
		/// Python call to decide where the walker should move to next.
		/// </summary>
		/// <param name="_walker">The Point which may be moved</param>
		/// <returns>[X Y Z] step to be added to the walkers position</returns>
		DirectX::XMFLOAT3 OnStep(const Point& _point);

		PyObject* m_pStepFun = nullptr;

		void OnStart();

		PyObject* m_pOnStartFun = nullptr;
	};
}