
#include <lua/lua.hpp>
#include <iostream>
#include <exception>
#include <chrono>
#include <vector>

struct Score{
	float like, comment, follow;
};

float score1(Score const& u){
	return u.like+u.follow/u.comment;
}	
const char* stmt1 = "like+follow+comment";

float score2(Score const& u){
	return u.like*u.follow/(u.comment-u.follow)*(u.like+u.follow)-0.1;
}	
const char* stmt2 = "like*follow/(comment-follow)*(like+follow)-0.1";

float score3(Score const& u){
	return (u.like+u.follow)*(u.like+u.comment)*(u.follow+u.comment)/(u.comment-u.follow)/(u.like-u.follow)/(u.like-u.comment);
}	
const char* stmt3 = "(like+follow)*(like+comment)*(follow+comment)/(comment-follow)/(like-follow)/(like-comment)";

void onError(lua_State* state, int err){
	if(err){  
		std::string err_msg = std::string("attr lua error with ret=") + std::to_string(err) + " and msg=" + lua_tostring(state, -1) ;      
		lua_pop(state, 1);	
		throw std::invalid_argument(err_msg);		
	}
}

class LuaEval{
	public:
		LuaEval(lua_State* s, std::string const& fn) : state(s), fnName(fn){} 	

		float operator()(struct Score const& score) const{
			lua_getglobal(state, fnName.c_str());
			lua_pushnumber(state, score.like);
			lua_pushnumber(state, score.comment);
			lua_pushnumber(state, score.follow);
			int err = lua_pcall(state, 3, 1, 0);

			onError(state, err);
			float result = lua_tonumber(state, -1);
			lua_pop(state, 1);	
			return result;
		}

	private:	
		lua_State* 	state;
		std::string 	fnName;
};

class LuaGrammar{
	public:
		LuaGrammar() : state(luaL_newstate()){}

		LuaEval Eval(std::string const& fnName, std::string const& stmt) {
			char fn_def[256];
			snprintf(fn_def, sizeof(fn_def), "function %s(like, comment, follow)\n return %s\n end", fnName.c_str(), stmt.c_str());
			int err = luaL_loadstring(state, fn_def) || lua_pcall(state, 0, 0, 0);
			onError(state, err);

			return {state, fnName};
		}

		~LuaGrammar(){
			lua_close(state);
		}
		
	private:	
		lua_State *state;
};



int main(int argc, char* argv[]){
	std::vector<Score> users = {{1,2,3},{3,90,876},{1223343,6787545,3453432}};

	LuaGrammar grammar;
	auto user_eval1 = grammar.Eval("s1", stmt1); 
	auto user_eval2 = grammar.Eval("s2", stmt2); 
	auto user_eval3 = grammar.Eval("s3", stmt3); 

	float f=0;
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	for(int i=0; i<1000000; ++i){
		for(auto& u : users){
			f += score1(u)	+ score2(u) + score3(u);		
		}
	}	
	auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
	std::cout << "raw cpp function took " << cost << "ms, result=" <<  f <<'\n';

	f=0.f;
	now = std::chrono::system_clock::now();
	for(int i=0; i<1000000; ++i){
		for(auto& u : users){
			f += user_eval1(u)	+ user_eval2(u) + user_eval3(u);		
		}
	}	
	cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
	std::cout << "lua parsed function took " << cost << "ms, result=" <<  f <<'\n';

	/*
	struct Score score={1, 2, 3};
	const char* stmt = "like+comment+follow";
	auto eval = grammar.Eval("weighted_score", stmt);
	std::cout << "score: like:" << score.like << ", comment:" << score.comment << ",follow:" <<  score.follow << '\n';
	std::cout << "stmt:" << stmt << "\n";
	std::cout << "result:" << eval(score)<< "\n";
	*/
	return 0;
}


