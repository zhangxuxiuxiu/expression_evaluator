#pragma once

#include <boost/variant/apply_visitor.hpp> // apply_visitor

#include <type_traits>
#include <exception>
#include <iostream>
#include "raw_ast.h"

namespace expr{
	namespace vm{

		template<class Functor>
		struct StackSize
		{
			typedef uint32_t result_type;

			result_type operator()(ast::Nil) const { BOOST_ASSERT(0); return 0; }

			result_type operator()(float n) const { 
				static_assert(sizeof(float)%sizeof(uint32_t)==0, "sizeof(float)%sizeof(uint32_t)==0");
				// op_int + float
				return 1+ sizeof(float)/sizeof(uint32_t); 
			}

			result_type operator()(ast::ScoreFn const& fn) const{
				static_assert(sizeof(Functor)%sizeof(uint32_t)==0, "sizeof(Functor)%sizeof(uint32_t)==0");
				// op_fn + fn
				return 1 + sizeof(Functor)/sizeof(uint32_t); 
			}

			result_type operator()(ast::Operand const& op) const{
				return boost::apply_visitor(*this,op);
			}

			result_type operator()(ast::Signed const& x) const{
				return (*this)(x.operand) + (x.sign=='-'? 1 : 0);
			}

			result_type operator()(ast::Program const& x) const{
				result_type size = (*this)(x.first);				
				for(ast::Operation const& oper : x.rest){
					size += (*this)(oper.operand) + 1;
				}
				return size;
			}
		};

		enum ByteCode
		{
			op_neg,     //  negate the top stack entry
			op_add,     //  add top two stack entries
			op_sub,     //  subtract top two stack entries
			op_mul,     //  multiply top two stack entries
			op_div,     //  divide top two stack entries

			//	op_load,    //  load a variable
			//	op_store,   //  store a variable
			op_int,     //  push constant integer into the stack
			//	op_stk_adj, //  adjust the stack for local variables
			op_fn	    //  call fn on user data
		};

		template<class Functor, class Item>
		class VirtualMachine{
			public:	
				VirtualMachine(uint32_t* code, uint32_t csize, uint32_t vsize): code_stack(code), 
				code_size(csize), var_stack(new float[vsize]), end_var(var_stack+vsize){}

				float Eval(Item const& item) const{
					uint32_t* pc = code_stack;
					float* stack_ptr = var_stack;
					const uint32_t * end_pc = code_stack + code_size;
					while (pc < end_pc)
					{
						switch (*pc++)
						{
							case op_neg:
								stack_ptr[-1] = -stack_ptr[-1];
								break;

							case op_add:
								--stack_ptr;
								stack_ptr[-1] += stack_ptr[0];
								break;

							case op_sub:
								--stack_ptr;
								stack_ptr[-1] -= stack_ptr[0];
								break;

							case op_mul:
								--stack_ptr;
								stack_ptr[-1] *= stack_ptr[0];
								break;

							case op_div:
								--stack_ptr;
								stack_ptr[-1] /= stack_ptr[0];
								break;

							case op_int:
								*stack_ptr++ = *(float*)(pc);
								pc += sizeof(float)/sizeof(uint32_t);
								break;

							case op_fn:
								*stack_ptr++ = expr::ast::EvalFn(*(Functor*)(pc), const_cast<Item *>(&item));
								pc += sizeof(Functor)/sizeof(uint32_t);
								break;
							default:
								throw std::invalid_argument("invalid ByteCode op");
						}
					}

					if(stack_ptr-1 != var_stack){
						throw std::invalid_argument("invalid ByteCode Eval");					
					}
					return *var_stack; 
				}
	
				template<class T>
				auto destruct(T* p)
				-> typename std::enable_if<std::is_trivial<T>::value>::type{
				}
				template<class T>
				auto destruct(T* p)
				-> typename std::enable_if<!std::is_trivial<T>::value && std::is_copy_constructible<T>::value>::type{
					p -> ~T();	
				}

				~VirtualMachine() {
					uint32_t* pc = code_stack;
					const uint32_t * end_pc = code_stack + code_size;
					while (pc < end_pc)
					{
						switch (*pc++)
						{
							case op_neg:
							case op_add:
							case op_sub:
							case op_mul:
							case op_div:
								break;
							case op_int:
								pc += sizeof(float)/sizeof(uint32_t);
								break;
							case op_fn:
								destruct((Functor*)(pc));
								pc += sizeof(Functor)/sizeof(uint32_t);
								break;
						}
					}
					delete [] code_stack;
					delete [] var_stack;
				}

			private:
				uint32_t* code_stack;
				uint32_t  code_size;
				float* 	  var_stack;
				float*    end_var;
		};

		template<class Functor, class Item>
		struct Compiler 
		{
			typedef void result_type;

			result_type operator()(ast::Nil) { BOOST_ASSERT(0);  }

			result_type operator()(float n) { 
				code_stack[pc++] = ByteCode::op_int;
				*(float*)(code_stack+pc) = n ;
				pc += sizeof(float)/sizeof(uint32_t);
			}

			template<class T>
			auto assign(T* dest, T const& v)
			-> typename std::enable_if<std::is_trivial<T>::value>::type{
				*dest = v;	
			}
			template<class T>
			auto assign(T* dest, T const& v)
			-> typename std::enable_if<!std::is_trivial<T>::value && std::is_copy_constructible<T>::value>::type{
				new (dest) T(v);	
			}

			result_type operator()(ast::ScoreFn const& fn) {
				Functor f = boost::any_cast<Functor>(fn);
				code_stack[pc++] = ByteCode::op_fn;
				assign((Functor*)(code_stack+pc), f);
				pc += sizeof(Functor)/sizeof(uint32_t);
			}

			result_type operator()(ast::Operand const& op) {
				return boost::apply_visitor(*this,op);
			}

			result_type operator()(ast::Signed const& x) {
				(*this)(x.operand);
				if(x.sign=='-'){
					code_stack[pc++] = ByteCode::op_neg;
				}
			}

			result_type operator()(ast::Program const& x) {
				if(code_stack == nullptr){
					stack_capacity = StackSize<Functor>()(x);
					code_stack = new uint32_t[stack_capacity];
				}

				(*this)(x.first);				
				for(ast::Operation const& oper : x.rest){
					(*this)(oper.operand);
					switch (oper.sign)
					{
						case '+': code_stack[pc++] = ByteCode::op_add; break;
						case '-': code_stack[pc++] = ByteCode::op_sub; break;
						case '*': code_stack[pc++] = ByteCode::op_mul; break;
						case '/': code_stack[pc++] = ByteCode::op_div; break;
					}
				}
			}

			VirtualMachine<Functor, Item>* operator()() {
				if(pc != stack_capacity){
					throw std::invalid_argument("wrong stack size calculation");
				}
				auto vm = new VirtualMachine<Functor, Item>{code_stack, stack_capacity, 32};
				code_stack = nullptr;
				stack_capacity = 0;
				pc = 0;
				return vm;
			}

			private:
				uint32_t* code_stack = nullptr;
				uint32_t  stack_capacity = 0;
				uint32_t  pc = 0;
		};
	}

	template<class Functor, class Item>
	class VMEvaluator{
		public:
			using element_type = Item; 
			VMEvaluator(expr::ast::Program const& prog){
				vm::Compiler<Functor, Item> compiler;
				compiler(prog);
				vm_ptr = compiler();
			} 

			float operator()(element_type const& item) const{
				return vm_ptr->Eval(item);
			}

			~VMEvaluator() { delete vm_ptr; }

		private:
			vm::VirtualMachine<Functor, Item> * vm_ptr; 
	};
}
