
#include <lua/lua.hpp>
#include <exception>
#include <memory> // shared_ptr
#include <vector>
#include <boost/algorithm/string/join.hpp>

#include "eval_fn.h"


namespace expr{

	void OnError(lua_State* state, int err){
		if(err){  
			std::string err_msg = std::string("attr lua error with ret=") + std::to_string(err) + " and msg=" + lua_tostring(state, -1) ;      
			lua_pop(state, 1);	
			throw std::invalid_argument(err_msg);		
		}
	}

	template<class Functor, class Evalee>
	class LuaEval{
		public:
			LuaEval(std::shared_ptr<lua_State> s, std::string const& fn, std::vector<Functor> fs) 
				: state(s), fnName(fn), functors(std::move(fs)){} 	

			float operator()(Evalee const& ee) const{
				lua_State* p = state.get();
				lua_getglobal(p, fnName.c_str());
				for(auto& f : functors){
					lua_pushnumber(p, ast::EvalFn(f, const_cast<Evalee *>(&ee)));
				}

				int err = lua_pcall(p, functors.size(), 1, 0);
				OnError(p, err);
				float result = lua_tonumber(p, -1);
				lua_pop(p, 1);	
				return result;
			}

		private:	
			std::shared_ptr<lua_State> 	state;
			std::string 			fnName;
			std::vector<Functor>		functors;
	};

	template<class Functor, class Evalee>
	class LuaGrammar{
		public:
			template<class Functors>
			LuaGrammar(std::vector<std::string> const& keys, Functors&& fs) : 
				state(luaL_newstate(),[](lua_State* p){lua_close(p);}), 
				keywords(boost::join(keys,",")), functors(std::forward<Functors>(fs)){}

			LuaEval<Functor, Evalee> Eval(std::string const& fnName, std::string const& stmt) {
				char fn_def[256];
				if(sizeof(fn_def) <= snprintf(fn_def, sizeof(fn_def), 
					"function %s(%s)\n return %s\n end", fnName.c_str(), keywords.c_str(), stmt.c_str())){
					throw std::invalid_argument("fn_def buffer size is too short");
				}
				auto p = state.get();
				int err = luaL_loadstring(p, fn_def) || lua_pcall(p, 0, 0, 0);
				OnError(p, err);

				return {state, fnName, functors};
			}

		private:	
			std::shared_ptr<lua_State> 	state;
			std::string 			keywords;
			std::vector<Functor>		functors;
	};


	template<class Functors, class Functor = typename Functors::value_type>
	auto MakeLuaGrammar(std::vector<std::string> const& keys, Functors const& functors)
	-> typename std::enable_if<arg1_type<Functor>::value, LuaGrammar<Functor, typename arg1_type<Functor>::type>>::type{
		if(keys.size()!=functors.size())  throw std::invalid_argument("keys must has the same size with functors");
		return {keys, functors};
	}

}
