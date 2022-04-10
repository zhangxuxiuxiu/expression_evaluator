#pragma once

#include <type_traits>

namespace expr{
	namespace ast{
	
		template<class Functor, class Item>
		auto EvalFn(Functor fn, Item*  u) 
		->typename std::enable_if<std::is_member_object_pointer<Functor>::value, float>::type { 
			return u->*fn; 
		}
		
		template<class Functor, class Item>
		auto EvalFn(Functor fn, Item*  u) 
		->typename std::enable_if<std::is_member_function_pointer<Functor>::value, float>::type  { 
			return (u->*fn)(); 
		}
		
		template<class Functor, class Item>
		auto EvalFn(Functor fn, Item*  u) 
		->typename std::enable_if<!std::is_member_pointer<Functor>::value, float>::type  { 
			return fn(*u); 
		}

	}
}
