#pragma once

#include <boost/function_types/parameter_types.hpp>
#include <boost/spirit/include/qi.hpp> // phrase_parse
#include <exception> //invalid_argument

#include "grammar.h"
#include "expr_evaluator.h"

namespace expr{

	template<class FuncType, class ItemType>
	class ItemEvaluator{
		public:
			using element_type = ItemType; 
			ItemEvaluator(ast::Program const& prog) : program(prog){}

			float operator()(element_type const& item) const{
				eval.item_ptr = &item;
				return eval(program);
			}

		private:
			expr::ast::Program program;
			expr::ast::ExprEvaluator<FuncType, element_type> eval;
			
	};

	template<class F>
	struct parameter_types{
		using arg1_type = typename boost::remove_cv<
						typename boost::remove_reference<
							typename boost::mpl::at_c<
								typename boost::function_types::parameter_types<F>::type, 0>
							::type
						>::type
					>::type;
	};

	// infer ItemType
	template<class StrType, class FuncType, class ItemType=typename parameter_types<FuncType>::arg1_type>
	ItemEvaluator<FuncType, ItemType>  Parse(StrType const& statement, CalcGrammar<typename StrType::const_iterator, FuncType> const& gram){
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

	// specify ItemType
	template<class ItemType, class StrType, class Grammar>
	auto Parse(StrType const& statement, Grammar const& gram)
	-> decltype(Parse<StrType, typename GrammarTraits<Grammar>::func_type, ItemType>(statement, gram)){
		return Parse<StrType, typename GrammarTraits<Grammar>::func_type, ItemType>(statement, gram);
	}

}
