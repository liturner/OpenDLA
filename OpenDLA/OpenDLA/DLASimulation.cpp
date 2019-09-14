#include "DLASimulation.h"

OpenDLA::DLASimulation::DLASimulation()
{
	Point a;
	a.pos = DirectX::XMFLOAT3(0, 0, 0);
	a.color = DirectX::XMFLOAT3(1, 0, 0);

	m_points.push_back(a);
}

OpenDLA::DLASimulation::~DLASimulation()
{
	Py_DECREF(m_pModule);
}

bool OpenDLA::DLASimulation::Initialise()
{
	Py_Initialize();
	PyObject* pName = PyUnicode_DecodeFSDefault("simulation");
	m_pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (!m_pModule) return false;

	m_pUpdateFun = PyObject_GetAttrString(m_pModule, "OnUpdate");

	if (!m_pUpdateFun) return false;
	if (!PyCallable_Check(m_pUpdateFun))
	{
		Py_DECREF(m_pUpdateFun);
		return false;
	}

	return true;
}

void OpenDLA::DLASimulation::Update()
{
	PyObject* pUpdateArgs = PyTuple_New(2);

	// Pack the first arg
	PyObject* pValue = PyLong_FromLong(5);
	PyTuple_SetItem(pUpdateArgs, 0, pValue);
	
	// Pack the second arg
	pValue = PyLong_FromLong(10);
	PyTuple_SetItem(pUpdateArgs, 1, pValue);

	PyObject* returned = PyObject_CallObject(m_pUpdateFun, pUpdateArgs);
	Py_DECREF(pUpdateArgs);

	PyObject* returned1 = PyTuple_GetItem(returned, 0);
	PyObject* returned2 = PyTuple_GetItem(returned, 1);
	PyObject* returned3 = PyTuple_GetItem(returned, 2);

	long x = PyLong_AsLong(returned1);
	long y = PyLong_AsLong(returned2);
	long z = PyLong_AsLong(returned3);

	m_points[0].pos.x += x;
	m_points[0].pos.y += y;
	m_points[0].pos.z += z;
}

bool OpenDLA::DLASimulation::Collides(const Point& _point)
{
	return false;
}

