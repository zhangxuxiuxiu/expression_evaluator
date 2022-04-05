// ref: https://www.cnblogs.com/DeeLMind/p/7655968.html

#include <lua.hpp>

#include <iostream>  
#include <string.h>  
using namespace std;  

extern "c"{
	int  attr_at(lua_State* state){
		void* u = lua_tonumber(L, -1); 
	}

}
   
/*usage: function add(a,b) \n return a + b \n  end*/
int main(int argc, char* argv[])  
{  
	if(argc!=2){
		cerr << "Usage: " << argv[0] << " attr_define.lua \n";
		return -1;
	}
	const char* attr_lua = argv[1];

	lua_State *L = luaL_newstate();  
	//luaL_openlibs(L);

	int ret = luaL_dofile(attr_lua);
	if(ret){
		cout << "attr lua error with " << ret << '\n';
		return -2;
	}

	ret = luaL_loadstring(L,"function expt1(u)\n\treturn like(u) + comment(u)/follow(u)\nend"); 
	if(ret){
		cout << "string lua def error with " << ret << '\n';
		return -3;
	}

	bRet = lua_pcall(L,0,0,0);	
	if(bRet){
		cout << "string lua call error with " << bRet << '\n';
		return -4;
	}

	lua_getglobal(L, "expt1");        // 获取函数，压入栈中  
	lua_pushnumber(L, 10);          // 压入第一个参数  
	lua_pushnumber(L, 20);          // 压入第二个参数  
	int iRet= lua_pcall(L, 2, 1, 0);// 调用函数，调用完成以后，会将返回值压入栈中，2表示参数个数，1表示返回结果个数。  
	if (iRet)                       // 调用出错  
	{  
		const char *pErrorMsg = lua_tostring(L, -1);  
		cout << pErrorMsg << endl;  
		lua_close(L);  
		return -1;  
	}  
	if (lua_isnumber(L, -1))        //取值输出  
	{  
		double fValue = lua_tonumber(L, -1);  
		cout << "Result is " << fValue << endl;  
	}  
	lua_close(L);  

    return 0;  
}
