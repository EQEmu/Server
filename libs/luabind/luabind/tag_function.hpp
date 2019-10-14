// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_TAG_FUNCTION_081129_HPP
#define LUABIND_TAG_FUNCTION_081129_HPP

#include <luabind/config.hpp>
#include <luabind/detail/type_traits.hpp>
#include <luabind/lua_state_fwd.hpp>

namespace luabind {

	template <class Signature, class F>
	struct tagged_function
	{
		tagged_function(F f)
			: f(f)
		{}

		F f;
	};

	namespace detail
	{

		struct invoke_context;
		struct function_object;

#ifndef LUABIND_NO_INTERNAL_TAG_ARGUMENTS
		template <class Signature, class F, typename... PolicyInjectors>
		int invoke(
			lua_State* L, function_object const& self, invoke_context& ctx
			// std::bind operator() is nonconst... is that fixable?
			, tagged_function<Signature, F> /*const*/& tagged
			, Signature, meta::type_list<PolicyInjectors...> const& injectors)
		{
			return invoke(L, self, ctx, tagged.f, Signature(), injectors);
		}
#else
		//template< typename PolicyList, typename Signature, typename F>
		//inline int invoke(lua_State* L, function_object const& self, invoke_context& ctx, F& f)

		template < typename PolicyList, typename Signature, typename F >
		int invoke(lua_State* L, function_object const& self, invoke_context& ctx, tagged_function<Signature, F> /*const*/& tagged)
		{
			return invoke<PolicyList, Signature>(L, self, ctx, tagged.f);
		}
#endif

	} // namespace detail

	template <class Signature, class F>
	tagged_function<deduce_signature_t<Signature>, F >
		tag_function(F f)
	{
		return f;
	}

} // namespace luabind

# endif // LUABIND_TAG_FUNCTION_081129_HPP

