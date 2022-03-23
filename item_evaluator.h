#pragma once

#include <boost/spirit/include/qi.hpp> // phrase_parse
#include <exception> //invalid_argument

#include "user_score.h"
#include "grammar.h"
#include "visitor.h"

namespace expr{

	class ItemEvaluator{
		public:
			ItemEvaluator(ast::Program const& prog) : program(prog){}

			float operator()(biz::UserScore const& user) const{
				eval.user_ptr = &user;
				return eval(program);
			}

		private:
			expr::ast::Program program;
			expr::ast::ExprEvaluator eval;
			
	};

	template<class StrType>
	ItemEvaluator  Parse(StrType const& statement, CalcGrammer<typename StrType::const_iterator> const& gram){
		expr::ast::Program program;
		auto iter = statement.begin();
		auto end = statement.end();
		boost::spirit::ascii::space_type space;
		bool r = boost::spirit::qi::phrase_parse(iter, end, gram, space, program);
		if (r && iter == end){
			return {program}; 
		}
		throw std::invalid_argument("invalid statement:"+statement);
	}

}
