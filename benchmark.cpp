
// Spirit v2.5 allows you to suppress automatic generation
// of predefined terminals to speed up complation. With
// BOOST_SPIRIT_NO_PREDEFINED_TERMINALS defined, you are
// responsible in creating instances of the terminals that
// you need (e.g. see qi::uint_type uint_ below).
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#include <string>
#include <vector>
#include <chrono>
#include <functional> //mem_fn

#include "grammar.h"
#include "vm_evaluator.h"
#include "raw_evaluator.h"
#include "lua_evaluator.h"

const char* stmt1 = "like+follow+comment";
const char* stmt2 = "like*follow/(comment-follow)*(like+follow)-0.1";
const char* stmt3 = "(like+follow)*(like+comment)*(follow+comment)/(comment-follow)/(like-follow)/(like-comment)";

namespace biz{
	struct UserScore{ float like; float follow; float comment;
		float lk() const{ return like;}
		float fw() const{ return follow;}
		float cmt() const{ return comment;}
		
		// required by lua eval 
		static constexpr const char* attr_list ="like,follow,comment";
		static float Eval(lua_State* state, const char* fn_name, UserScore const& score){
			lua_getglobal(state, fn_name);
			lua_pushnumber(state, score.like);
			lua_pushnumber(state, score.follow);
			lua_pushnumber(state, score.comment);
			int err = lua_pcall(state, 3, 1, 0);

			expr::OnError(state, err);
			float result = lua_tonumber(state, -1);
			lua_pop(state, 1);	
			return result;
		}
	};

	float score1(UserScore const& u){
		return u.like+u.follow/u.comment;
	}	
	float score2(UserScore const& u){
		return u.like*u.follow/(u.comment-u.follow)*(u.like+u.follow)-0.1;
	}	
	float score3(UserScore const& u){
		return (u.like+u.follow)*(u.like+u.comment)*(u.follow+u.comment)/(u.comment-u.follow)/(u.like-u.follow)/(u.like-u.comment);
	}	
}


int main()
{
	auto symbols = {"like","follow", "comment"};
	//auto fnList  = {&biz::UserScore::like,
	//	&biz::UserScore::follow,
	//	&biz::UserScore::comment
	//};
	auto fnList  = {&biz::UserScore::lk,
		&biz::UserScore::fw,
		&biz::UserScore::cmt
	};
	auto&& gram = expr::MakeGrammar(symbols, fnList);
	std::vector<biz::UserScore> users = {{1,2,3},{3,90,876},{1223343,6787545,3453432}};
	auto f = 0.f;

	{
		auto user_eval1 = gram.Parse(stmt1);	
		auto user_eval2 = gram.Parse(stmt2);	
		auto user_eval3 = gram.Parse(stmt3);	
	
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		for(int i=0; i<1000000; ++i){
			for(auto& u : users){
				f += biz::score1(u)	+ biz::score2(u) + biz::score3(u);		
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
		std::cout << "ast parsed function took " << cost << "ms, result=" <<  f <<'\n';
	}

	{
		auto user_eval1 = gram.Parse<expr::RawEvaluator>(stmt1);	
		auto user_eval2 = gram.Parse<expr::RawEvaluator>(stmt2);	
		auto user_eval3 = gram.Parse<expr::RawEvaluator>(stmt3);	
	
		f=0.f;
		auto now = std::chrono::system_clock::now();
		for(int i=0; i<1000000; ++i){
			for(auto& u : users){
				f += user_eval1(u)	+ user_eval2(u) + user_eval3(u);		
			}
		}	
		auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
		std::cout << "raw parsed function took " << cost << "ms, result=" <<  f <<'\n';
	}

	{
		auto user_eval1 = gram.Parse<expr::VMEvaluator>(stmt1);	
		auto user_eval2 = gram.Parse<expr::VMEvaluator>(stmt2);	
		auto user_eval3 = gram.Parse<expr::VMEvaluator>(stmt3);	
	
		f=0.f;
		auto now = std::chrono::system_clock::now();
		for(int i=0; i<1000000; ++i){
			for(auto& u : users){
				f += user_eval1(u)	+ user_eval2(u) + user_eval3(u);		
			}
		}	
		auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
		std::cout << "vm parsed function took " << cost << "ms, result=" <<  f <<'\n';
	}

	{
		expr::LuaGrammar<biz::UserScore> grammar;
		auto user_eval1 = grammar.Eval("s1", stmt1); 
		auto user_eval2 = grammar.Eval("s2", stmt2); 
		auto user_eval3 = grammar.Eval("s3", stmt3); 

		f=0.f;
		auto now = std::chrono::system_clock::now();
		for(int i=0; i<1000000; ++i){
			for(auto& u : users){
				f += user_eval1(u)	+ user_eval2(u) + user_eval3(u);		
			}
		}	
		auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
		std::cout << "lua parsed function took " << cost << "ms, result=" <<  f <<'\n';
	}
	return 0;
}

