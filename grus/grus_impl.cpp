#include "grus.h"
#include "grus_impl.h"
#include "jitmgr.h"
#include "utility.h"
#include "PackEngine.h"
#include "BlockStream.h"
#include "common.h"
#include "buildsystem.h"

static PyObject*
Grus_EnableDebug(PyObject* self, PyObject* args, PyObject* kwargs)
{
#if (WIN32)
	DWORD procId = GetCurrentProcessId();
	char info[1000];
	sprintf_s(info, 1000, "Attach to python process %d to debug,"
			"after attach,don't forget to close this Message Box"
			"to make program continue!", procId);
	MessageBox(NULL, info,"PyJit Native Debug", MB_OK);
#endif
	return PyLong_FromLong(0);
}
static PyObject*
Grus_main(PyObject* self, PyObject* args, PyObject* kwargs)
{
	printf("inside Grus_main\r\n");
	return PyLong_FromLong(0);
}
static PyObject*
Grus_register(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return JITManager::I().Register(args,kwargs);
}
static PyObject*
Grus_Init(PyObject* self, PyObject* args, PyObject* kwargs)
{
	JITManager::I().Init(args, kwargs);
	return PyLong_FromLong(0);
}

static PyObject*
Grus_Function(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return JITManager::I().CreateFuncWrapper(args, kwargs);
}
static PyObject*
Grus_Class(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return JITManager::I().CreateClassWrapper(args, kwargs);
}

static PyObject*
Grus_Generate(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return JITManager::I().Generate(args, kwargs);
}
static PyObject*
Grus_Build(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return BuildSystem::I().Build(args, kwargs);
}
static PyObject*
Grus_Expand(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyJit::Object oArgs(args);
	if (oArgs.GetCount() > 0)
	{
		std::string curModuleFolder = GetCallerPyModuleFolder();
		return (PyObject*)(GalaxyJitPtr)
			BuildSystem::I().ExpandWildCast(curModuleFolder,oArgs[0]);
	}
	else
	{
		Py_RETURN_NONE;
	}
}
static PyObject*
Grus_Generate_UID(PyObject* self, PyObject* args, PyObject* kwargs)
{
	return JITManager::I().Generate_UID(args, kwargs);
}
static PyObject*
Grus_Pack(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyObject* pStreamOb = nullptr;
	if (PyTuple_Check(args) && PyTuple_GET_SIZE(args) > 0)
	{
		PyObject* pObToPack = PyTuple_GetItem(args, 0);
		BlockStream stream;
		PackEngine pe;
		std::vector<PyObject*> obs{pObToPack};
		bool bOK = pe.Pack(stream, obs);
		if (bOK)
		{
			STREAM_SIZE size = stream.Size();
			pStreamOb = PyByteArray_FromStringAndSize(nullptr, size);
			char* buf = PyByteArray_AsString(pStreamOb);//get the buffer from pStreamOb
			stream.FullCopyTo(buf, size);
		}
	}
	if(pStreamOb == nullptr)
	{
		pStreamOb = Py_None;
		Py_IncRef(pStreamOb);
	}
	return pStreamOb;
}

static PyObject*
Grus_Unpack(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyObject* pOutOb = nullptr;
	if (PyTuple_Check(args) && PyTuple_GET_SIZE(args) > 0)
	{
		PyObject* pStreamOb = PyTuple_GetItem(args, 0);
		if (PyByteArray_Check(pStreamOb))
		{
			STREAM_SIZE size = PyByteArray_Size(pStreamOb);
			char* pData = PyByteArray_AsString(pStreamOb);
			BlockStream stream(pData, size, false);
			PackEngine pe;
			auto outList = pe.Unpack<PyObject*>(stream);
			if (outList.size() > 0)
			{
				pOutOb = outList[0];
				for (int i = 1; i < outList.size(); i++)
				{
					if (outList[i])
					{
						Py_DecRef(outList[i]);
					}
				}
			}
		}
	}
	if(pOutOb == nullptr)
	{
		pOutOb = Py_None;
		Py_IncRef(pOutOb);
	}
	return pOutOb;
}

PyMethodDef RootMethods[] =
{
	{	"main",
		(PyCFunction)Grus_main,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax grus.main(*args,**kwargs)"
	},
	{	"register",
		(PyCFunction)Grus_register,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax grus.register(*args,**kwargs)"
	},
	{	"Init",
		(PyCFunction)Grus_Init,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax grus.Init(*args,**kwargs)"
	},
	{	"EnableDebug",
		(PyCFunction)Grus_EnableDebug,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax wrapper = grus.EnableDebug(*args,**kwargs)"
	},
	{	"func", 
		(PyCFunction)Grus_Function,
		METH_VARARGS|METH_KEYWORDS,
		"Syntax: wrapper = grus.func(*args,**kwargs)" 
	},
	{	"object",
		(PyCFunction)Grus_Class,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax: wrapper = grus.object(*args,**kwargs)"
	},
	{	"generate",
		(PyCFunction)Grus_Generate,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.generate(*args,**kwargs)"
	},
	{	"build",
		(PyCFunction)Grus_Build,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.build(*args,**kwargs)"
	},
	{	"expand",
		(PyCFunction)Grus_Expand,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.expand(srcs)"
	},
	{	"generate_uid",
		(PyCFunction)Grus_Generate_UID,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.generate_uid(*args,**kwargs)"
	},
	{	"Pack",
		(PyCFunction)Grus_Pack,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.Pack(*args,**kwargs)"
	},
	{	"Unpack",
		(PyCFunction)Grus_Unpack,
		METH_VARARGS | METH_KEYWORDS,
		"Syntax:grus.Unpack(*args,**kwargs)"
	},
	{ NULL, NULL, 0, NULL }
};

