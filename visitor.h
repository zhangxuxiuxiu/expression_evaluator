#pragma once

#include <iostream> //cout

#include <boost/variant/apply_visitor.hpp> // apply_visitor

#include "ast.h"

namespace expr { namespace ast
{
	///////////////////////////////////////////////////////////////////////////
	//  The AST Printer
	///////////////////////////////////////////////////////////////////////////
	struct Printer
	{
		typedef void result_type;

		void operator()(Nil) const {}
		void operator()(float n) const { std::cout << n; }
		void operator()(ScoreFn const& fn ) const { std::cout << "score fn"; }

		void operator()(Operation const& x) const
		{
			boost::apply_visitor(*this, x.operand);
			switch (x.operator_)
			{
				case '+': std::cout << " add"; break;
				case '-': std::cout << " subt"; break;
				case '*': std::cout << " mult"; break;
				case '/': std::cout << " div"; break;
			}
		}

		void operator()(Signed const& x) const
		{
			boost::apply_visitor(*this, x.operand);
			switch (x.sign)
			{
				case '-': std::cout << " neg"; break;
				case '+': std::cout << " pos"; break;
			}
		}

		void operator()(Program const& x) const
		{
			boost::apply_visitor(*this, x.first);
			for(Operation const& oper : x.rest)
			{
				std::cout << ' ';
				(*this)(oper);
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////
	//  The AST evaluator
	///////////////////////////////////////////////////////////////////////////
	struct ExprEvaluator
	{
		mutable biz::UserScore const* user_ptr=nullptr;

		typedef float result_type;

		result_type operator()(Nil) const { BOOST_ASSERT(0); return 0; }
		result_type operator()(float n) const { return n; }
		result_type operator()(ScoreFn const& fn) const { return fn(*user_ptr); }

		result_type operator()(Operation const& x, float lhs) const
		{
			result_type rhs = boost::apply_visitor(*this, x.operand);
			switch (x.operator_)
			{
				case '+': return lhs + rhs;
				case '-': return lhs - rhs;
				case '*': return lhs * rhs;
				case '/': return lhs / rhs;
			}
			BOOST_ASSERT(0);
			return 0;
		}

		result_type operator()(Signed const& x) const
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

		result_type operator()(Program const& x) const
		{
			result_type state = boost::apply_visitor(*this, x.first);
			for(Operation const& oper : x.rest)
			{
				state = (*this)(oper, state);
			}
			return state;
		}
	};
}}
