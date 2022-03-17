
#include <Python.h>
#include <iostream>

using namespace std;


int main()
{
	string chdir_cmd = string("sys.path.append(\"") + "\")";    //Python模組在當前目錄下
	const char* cstr_cmd = chdir_cmd.c_str();

	PyObject* pRet;
	PyObject* pModule;
	PyObject *pFunc;

	Py_Initialize();  //初始化Python環境
	PyRun_SimpleString("import sys");  //執行Python語句
	PyRun_SimpleString(cstr_cmd); //新增Python模組路徑

	pModule = PyImport_ImportModule("python");
	if (!pModule)
	{
		cout << "Can't Find The Module,Please Check Module Name and try again!" << endl;
		return 0;
	}
	pFunc = PyObject_GetAttrString(pModule, "retNum");  //呼叫retNum函式，返回整型
	if (pFunc && PyCallable_Check(pFunc))
	{//為使程式碼緊湊，僅寫第一次，後面同上處理，忽略判斷
		pRet = PyObject_CallObject(pFunc, NULL);
		cout << PyLong_AsLong(pRet) << endl;
	}
	pFunc = PyObject_GetAttrString(pModule, "retDouble");  //呼叫retDouble，返回double型別
	pRet = PyObject_CallObject(pFunc, NULL);
	cout << PyFloat_AsDouble(pRet) << endl;


	pFunc = PyObject_GetAttrString(pModule, "retString");  //呼叫retString，返回ANSI字串
	pRet = PyObject_CallObject(pFunc, NULL);
	cout << PyUnicode_AsUTF8(pRet) << endl;


	pFunc = PyObject_GetAttrString(pModule, "retUString");  //呼叫retUString，返回Unicode字串
	pRet = PyObject_CallObject(pFunc, NULL);
	cout << PyBytes_AsString(pRet) << endl;

	pFunc = PyObject_GetAttrString(pModule, "retList");  //呼叫retList，返回陣列型別。
	pRet = PyObject_CallObject(pFunc, NULL);
	int val_num = PyList_Size(pRet);
	for (int i = 0; i < val_num; i++)
	{
		cout << PyLong_AsLong(PyList_GetItem(pRet, i)) << " ";
	}
	cout << endl;

	pFunc = PyObject_GetAttrString(pModule, "retTuple");  //呼叫retTuple，返回tuple型別
	int w = 0, h = 0;
	char *ret_str;
	pRet = PyObject_CallObject(pFunc, NULL);
	PyArg_ParseTuple(pRet, "iis", &w, &h, &ret_str);  //多個值必須一一對應
	cout << w << " " << h << " " << ret_str << endl;

	pFunc = PyObject_GetAttrString(pModule, "retDict"); //呼叫retDict，返回dict型別
	pRet = PyObject_CallObject(pFunc, NULL);
	PyObject* pKeys = PyDict_Keys(pRet);
	PyObject* pValues = pRet;
	int LongList = PyDict_Size(pRet);
	for (int i = 0; i < LongList; i++)
	{
		PyObject *pKey = PyList_GetItem(pKeys, i);
		PyObject *pValue = PyDict_GetItem(pValues, pKey);
		cout << PyLong_AsLong(pKey) << ":" << PyUnicode_AsUTF8(pValue) << endl;
	}

	Py_Finalize();
	system("pause");
    return 0;
}

