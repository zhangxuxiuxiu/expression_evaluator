
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

namespace biz{
	struct UserScore{ float like; float follow; float comment;
		float lk() const{ return like;}
		float fw() const{ return follow;}
		float cmt() const{ return comment;}
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
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Expression parser...\n\n";
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Type an expression...or [q or Q] to quit\n\n";

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
	auto user_eval1 = gram.Parse(std::string{"like+follow+comment"});	
	auto user_eval2 = gram.Parse(std::string{"like*follow/(comment-follow)*(like+follow)-0.1"});	
	auto user_eval3 = gram.Parse(std::string{"(like+follow)*(like+comment)*(follow+comment)/(comment-follow)/(like-follow)/(like-comment)"});	

	std::vector<biz::UserScore> users = {{1,2,3},{3,90,876},{1223343,6787545,3453432}};

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	auto f = 0.f;
	for(int i=0; i<100000; ++i){
		for(auto& u : users){
			f += biz::score1(u)	+ biz::score2(u) + biz::score3(u);		
		}
	}	
	auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
	std::cout << "raw function took " << cost << "ms, result=" <<  f <<'\n';

	f=0.f;
	now = std::chrono::system_clock::now();
	for(int i=0; i<100000; ++i){
		for(auto& u : users){
			f += user_eval1(u)	+ user_eval2(u) + user_eval3(u);		
		}
	}	
	cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-now).count();
	std::cout << "parsed function took " << cost << "ms, result=" <<  f <<'\n';

	return 0;
}

