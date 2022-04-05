#include <lua.hpp>

namespace expr{
	namespace lua {

		class LuaGrammar{
			public:
				LuaGrammar() : state(luaL_newstate()){}

				LuaGrammar& Eval 

				~LuaGrammar(){ lua_close(state); }
			private:
				luaL_state* state;
		};
		

	}



	LuaGrammar MakeLuaGrammar(){
		return {};
	}
}
