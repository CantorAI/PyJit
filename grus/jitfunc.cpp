#include "jitfunc.h"
#include "utility.h"
#include "jitfuncinfo.h"

static PyObject*
JitFunc_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PyJitFunc* self = (PyJitFunc*)type->tp_alloc(type, 0);
	self->funcInfo = new JitFuncInfo();
	return (PyObject*)self;
}
static int
JitFunc_init(PyJitFunc* self, PyObject* args, PyObject* kwds)
{
	return 0;
}
static void JitFunc_dealloc(PyJitFunc* self)
{
	if (self->funcInfo)
	{
		delete self->funcInfo;
		self->funcInfo = nullptr;
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject JitFuncType = {
	PyObject_HEAD_INIT(&PyType_Type)
	"JitFunc",             /*tp_name*/
	sizeof(PyJitFunc),             /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)JitFunc_dealloc, /*tp_dealloc*/
	0,/*tp_vectorcall_offset*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_as_async*/

	0,                         /*tp_repr*/

	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/

	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/

	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"JitFunc objects",           /* tp_doc */

	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	0,             /* tp_methods */
	0,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)JitFunc_init,      /* tp_init */
	0,                         /* tp_alloc */
	JitFunc_new,                 /* tp_new */
	0,//tp_free
	0,//tp_is_gc
	0,//tp_bases
	0,//tp_mro
	0,//tp_cache
	0,//tp_subclasses
	0,//tp_weaklist
	0,//tp_del
	0,//tp_version_tag
	0,//tp_finalize
	0,//tp_vectorcall
};

static bool __JitFuncType_Prepared = false;
void PrepareJitFuncType()
{
	PyType_Ready(&JitFuncType);
	__JitFuncType_Prepared = true;
}

PyJitFunc* NewPyJitFunc()
{
	if (!__JitFuncType_Prepared)
	{
		PrepareJitFuncType();
	}
	auto self = (PyJitFunc*)JitFuncType.tp_alloc(&JitFuncType, 0);
	self->funcInfo = new JitFuncInfo();
	return self;
}
void DestroyPyJitFunc(PyJitFunc* self)
{
	if (self->funcInfo)
	{
		delete self->funcInfo;
		self->funcInfo = nullptr;
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}