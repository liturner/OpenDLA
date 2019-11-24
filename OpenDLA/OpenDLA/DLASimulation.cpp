#include "DLASimulation.h"

OpenDLA::DLASimulation::DLASimulation()
{
	
}

OpenDLA::DLASimulation::~DLASimulation()
{
	Destroy();
	Py_Finalize();
}

bool OpenDLA::DLASimulation::Initialise()
{
	PyMethodDef OpenDLA_PyMethodDef[] = {
		{ "addPoint", &AddPoint, METH_VARARGS, "Initialises the SDK using the default arguments." },
		{ NULL, NULL, 0, NULL }
	};

	OpenDLA::DLASimulation::OpenDLA_PyModuleDef = {
		PyModuleDef_HEAD_INIT,
		"opendla",
		NULL,
		-1,
		OpenDLA_PyMethodDef,
		NULL, NULL, NULL, NULL
	};

	PyImport_AppendInittab( "opendla", &InitialisePythonAPI);


	Py_Initialize();
	PyObject* pName = PyUnicode_DecodeFSDefault("simulation");
	m_pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (!m_pModule) return false;

	// It is not a failure if the function is not there, but it is a fail
	// if the function is there but not callable
	m_pStepFun = PyObject_GetAttrString(m_pModule, "OnStep");
	if (m_pStepFun && !PyCallable_Check(m_pStepFun))
	{
		Destroy();
		return false;
	}

	m_pOnStartFun = PyObject_GetAttrString(m_pModule, "OnStart");
	if (m_pOnStartFun && !PyCallable_Check(m_pOnStartFun))
	{
		Destroy();
		return false;
	}






	// Finally
	OnStart();

	return true;
}

void OpenDLA::DLASimulation::Update()
{
	/*
	Logic:
	1. Move the walker
	2. Check for Collision
	3. Save walker as point if needed
	4. Spawn new Walker
	*/

	// 1
	for (int i = 0; i < m_walkers.size(); i++)
	{
		OnStep(m_points[m_walkers[0]]);
		DirectX::XMFLOAT3 move = OnStep(m_points[m_walkers[0]]);
		m_points[m_walkers[0]].pos.x = m_points[m_walkers[0]].pos.x + move.x;
		m_points[m_walkers[0]].pos.y = m_points[m_walkers[0]].pos.y + move.y;
		m_points[m_walkers[0]].pos.z = m_points[m_walkers[0]].pos.z + move.z;

		// 2
		if (Collides(m_points[m_walkers[0]]))
		{
			m_points.push_back(m_points[m_walkers[0]]);
			m_walkers[0]++;
		}
	}
}

void OpenDLA::DLASimulation::Destroy()
{
	if(m_pStepFun) Py_DECREF(m_pStepFun);
	if(m_pOnStartFun) Py_DECREF(m_pOnStartFun);
	Py_DECREF(m_pModule);
}

bool OpenDLA::DLASimulation::Collides(const Point& _point)
{
	return false;
}

PyObject* OpenDLA::DLASimulation::InitialisePythonAPI(void)
{
	return PyModule_Create(&OpenDLA_PyModuleDef);
}

PyObject* OpenDLA::DLASimulation::AddPoint(PyObject* self, PyObject* args)
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 colour = DirectX::XMFLOAT3(1, 1, 1);

	if (!PyArg_ParseTuple(args, "(ddd)|(ddd)", &position.x, &position.y, &position.z, &colour.x, &colour.y, &colour.z))
	{
		PyErr_Print();
		Py_RETURN_FALSE;
	}

	Py_RETURN_NONE;
}

DirectX::XMFLOAT3 OpenDLA::DLASimulation::OnStep(const Point& _point)
{
	PyObject* returned = PyObject_CallObject(m_pStepFun, nullptr);
/*

	PyObject* returned1 = PyTuple_GetItem(returned, 0);
	PyObject* returned2 = PyTuple_GetItem(returned, 1);
	PyObject* returned3 = PyTuple_GetItem(returned, 2);

	float x = PyFloat_AsDouble(returned1);
	float y = PyFloat_AsDouble(returned2);
	float z = PyFloat_AsDouble(returned3);

	return DirectX::XMFLOAT3(x, y, z);*/

	if(returned) Py_DECREF(returned);
	return DirectX::XMFLOAT3(0, 0, 0);
}

void OpenDLA::DLASimulation::OnStart()
{
	if (m_pOnStartFun != nullptr)
	{
		// It is up to the user to define the simulation logic. They may not
		// define a seed, but may investigate two walkers colliding with one 
		// another
		PyObject_CallObject(m_pOnStartFun, nullptr);
	}
	else
	{
		// If the user has not defined an OnStart function, we just initialise
		// to a single [0 0 0] seed.
		Point a;
		a.pos = DirectX::XMFLOAT3(0, 0, 0);
		a.color = DirectX::XMFLOAT3(1, 0, 0);

		m_points.push_back(a);

		a.pos = DirectX::XMFLOAT3(10, 0, 0);
		a.color = DirectX::XMFLOAT3(0, 1, 1);

		m_points.push_back(a);

		m_walkers.push_back(1);
	}
}

