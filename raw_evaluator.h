#pragma once

#include "raw_ast.h"

namespace expr{

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	template<class Functor, class Item>
	struct RawTransformer
	{
		// Item can't be qualified with const here in case non-const object or function
		mutable  Item * item_ptr=nullptr;

		typedef float result_type;

		result_type operator()(ast::Nil) const { BOOST_ASSERT(0); return 0; }

		result_type operator()(float n) const { return n; }

		template<class F=Functor>
		result_type operator()(ast::ScoreFn const& fn) const{
			return ast::EvalFn(boost::any_cast<Functor>(fn), item_ptr); 
		}

		result_type operator()(ast::Operation const& x, float lhs) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand);
			switch (x.sign)
			{
				case '+': return lhs + rhs;
				case '-': return lhs - rhs;
				case '*': return lhs * rhs;
				case '/': return lhs / rhs;
			}
			BOOST_ASSERT(0);
			return 0;
		}

		result_type operator()(ast::Signed const& x) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand);
			switch (x.sign)
			{
				case '-': return -rhs;
				case '+': return +rhs;
			}
			BOOST_ASSERT(0);
			return 0;
		}

		result_type operator()(ast::Program const& x) const
		{
			result_type state = boost::apply_visitor(*this, x.first);
			for(ast::Operation const& oper : x.rest)
			{
				state = (*this)(oper, state);
			}
			return state;
		}
	};

	template<class Functor, class Item>
	class RawEvaluator{
		public:
			using element_type = Item; 
			RawEvaluator(ast::Program const& prog) : program(prog){}

			float operator()(element_type const& item) const{
				eval.item_ptr = const_cast<Item*>(&item);
				return eval(program);
			}

		private:
			ast::Program program;
			RawTransformer<Functor, element_type> eval;
			
	};

}
