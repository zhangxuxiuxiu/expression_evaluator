#pragma once

#include "expr_evaluator.h"
#include "ast.h"

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

			float operator()(element_type & item) {
				eval.item_ptr = &item;
				return eval(program);
			}

		private:
			expr::ast::Program program;
			expr::ast::ExprEvaluator<FuncType, element_type> eval;
			
	};

}
