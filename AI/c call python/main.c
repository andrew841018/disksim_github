#include<Python.h>
#include<stdio.h>
#include<stdlib.h>
int complex_calc(int x, int a, int b) {
    PyObject *pModule, *pFunc, *pArgs, *pKargs, *pRes;
    int res;

    // 匯入 `speedup_dev` module & `complex_calc` function 
    pModule = PyImport_Import(PyUnicode_FromString("test"));
    pFunc = PyObject_GetAttrString(pModule, "complex_calc");

    // 把 arguments 打包成 PyObject
    pArgs = Py_BuildValue("(i)", x);
    pKargs = Py_BuildValue("{s:i, s:i}", "a", a, "b", b);

    // 呼叫 function 
    pRes = PyObject_Call(pFunc, pArgs, pKargs);

    // 檢查有沒有呼叫失敗，沒有的話把回傳值從 PyObject 包回 C type 
    if (pRes == NULL)
        return -1;
   // res = PyLong_AsLong(pRes);
   res=PyLong_AsLong(pRes);
    // 處理一下 memory allocation 
    Py_DECREF(pModule);
    Py_DECREF(pFunc);
    Py_DECREF(pArgs);
    Py_DECREF(pKargs);
    Py_DECREF(pRes);

    return res;
}
int main() {
    // Initialize Python 直譯器
    Py_Initialize();

    // 設置 Python search path
    PySys_SetPath(Py_DecodeLocale(".", NULL));

    // 呼叫 function
    printf("%d\n", complex_calc(3, 2, 4));

    // 釋放 Python 直譯器佔用資源
    Py_Finalize();
}
