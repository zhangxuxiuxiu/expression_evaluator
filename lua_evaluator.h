
#include <lua/lua.hpp>
#include <exception>


namespace expr{

	void OnError(lua_State* state, int err){
		if(err){  
			std::string err_msg = std::string("attr lua error with ret=") + std::to_string(err) + " and msg=" + lua_tostring(state, -1) ;      
			lua_pop(state, 1);	
			throw std::invalid_argument(err_msg);		
		}
	}

	template<class Evalee>
	class LuaEval{
		public:
			LuaEval(lua_State* s, std::string const& fn) : state(s), fnName(fn){} 	

			float operator()(Evalee const& ee) const{
				return Evalee::Eval(state, fnName.c_str(), ee);
			}

		private:	
			lua_State* 	state;
			std::string 	fnName;
	};

	template<class Evalee>
	class LuaGrammar{
		public:
			LuaGrammar() : state(luaL_newstate()){}

			LuaEval<Evalee> Eval(std::string const& fnName, std::string const& stmt) {
				char fn_def[256];
				snprintf(fn_def, sizeof(fn_def), "function %s(%s)\n return %s\n end", fnName.c_str(), Evalee::attr_list, stmt.c_str());
				int err = luaL_loadstring(state, fn_def) || lua_pcall(state, 0, 0, 0);
				OnError(state, err);

				return {state, fnName};
			}

			~LuaGrammar(){
				lua_close(state);
			}

		private:	
			lua_State *state;
	};

}
