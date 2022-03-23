#pragma once

#include <boost/spirit/include/qi.hpp> // phrase_parse
#include <exception> //invalid_argument

#include "user_score.h"
#include "grammar.h"
#include "visitor.h"

namespace expr{

	class ItemEvaluator{
		public:
			float operator()(biz::UserScore const& user) const{
				eval.user_ptr = &user;
				return eval(program);
			}

		private:
			expr::ast::Program program;
			expr::ast::ExprEvaluator eval;
			
		public:
			template<class StrType>
			static ItemEvaluator  Parse(StrType const& statement, CalcGrammer<typename StrType::const_iterator> const& gram){
				ItemEvaluator evaluator;
				auto iter = statement.begin();
				auto end = statement.end();
				boost::spirit::ascii::space_type space;
				bool r = boost::spirit::qi::phrase_parse(iter, end, gram, space, evaluator.program);
				if (r && iter == end){
					return evaluator; 
				}
				throw std::invalid_argument("invalid statement:"+statement);
			}
	};



}
