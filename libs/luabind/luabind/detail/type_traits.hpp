// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef LUABIND_TYPETRAITS_HPP_INCLUDED
#define LUABIND_TYPETRAITS_HPP_INCLUDED

#include <type_traits>
#include <functional>				// reference_wrapper
#include <luabind/detail/meta.hpp>

namespace luabind {

	namespace detail {

		template< typename T >
		struct is_const_reference
			: public std::conditional< std::is_reference<T>::value && std::is_const<typename std::remove_reference<T>::type>::value, std::true_type, std::false_type >::type
		{
		};

		template<class T>
		struct is_nonconst_reference
			: public std::conditional< std::is_reference<T>::value && !std::is_const<typename std::remove_reference<T>::type>::value, std::true_type, std::false_type >::type
		{
		};

		template<class T>
		struct is_const_pointer
			: public std::conditional< std::is_const<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value, std::true_type, std::false_type >::type
		{
		};

		template<class T>
		struct is_nonconst_pointer :
			public std::conditional < std::is_pointer<T>::value && !std::is_const<typename std::remove_pointer<T>::type>::value, std::true_type, std::false_type >::type
		{
		};

		template<int v1, int v2>
		struct max_c
		{
			enum { value = (v1 > v2) ? v1 : v2 };
		};

	} // namespace detail

	template< typename T >
	struct remove_const_reference {
		using type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
	};

	template< typename T >
	using remove_const_reference_t = typename remove_const_reference<T>::type;

	//
	// most_derived
	//

	template<class A, class B>
	struct most_derived
	{
		using type = typename std::conditional<
			std::is_base_of<A, B>::value
			, B
			, A
		>::type;
	};

	template< typename A, typename B >
	using most_derived_t = typename most_derived<A, B>::type;

	//
	// null_type
	//

	struct null_type {};

	template< typename T >
	struct is_null_type : public std::false_type {};

	template< >
	struct is_null_type< null_type > : public std::true_type {};

	//
	// deduce_signature
	//

	template< typename, typename > struct tagged_function;

	template< typename T, typename WrappedType = null_type >
	struct deduce_signature;

	template< typename R, typename... Args, typename WrappedType >
	struct deduce_signature < R(Args...), WrappedType >
	{
		using type = meta::type_list< R, Args... >;
	};

	template< typename R, typename... Args, typename WrappedType >
	struct deduce_signature < R(*)(Args...), WrappedType >
	{
		using type = meta::type_list< R, Args... >;
	};

	template< typename R, typename Class, typename... Args >
	struct deduce_signature < R(Class::*)(Args...), null_type >
	{
		using type = meta::type_list< R, Class&, Args... >;
	};

	template< typename R, typename Class, typename... Args >
	struct deduce_signature < R(Class::*)(Args...) const, null_type >
	{
		using type = meta::type_list< R, Class const&, Args... >;
	};

	template< typename R, typename Class, typename... Args, class WrappedType >
	struct deduce_signature < R(Class::*)(Args...), WrappedType >
	{
		using type = meta::type_list< R, typename most_derived<Class, WrappedType>::type&, Args... >;
	};

	template< typename R, typename Class, typename... Args, class WrappedType >
	struct deduce_signature < R(Class::*)(Args...) const, WrappedType >
	{
		using type = meta::type_list< R, typename most_derived<Class, WrappedType>::type const&, Args... >;
	};

	template< typename Signature, typename F, class WrappedType >
	struct deduce_signature< tagged_function< Signature, F >, WrappedType >
	{
		using type = Signature;
	};

	template< typename T, typename WrappedType = null_type >
	using deduce_signature_t = typename deduce_signature<T, WrappedType>::type;

	//
	// is_reference_wrapper
	//

	template< typename T > struct is_reference_wrapper : public std::false_type { };
	template< typename T > struct is_reference_wrapper< std::reference_wrapper<T> > : public std::true_type { };

	//
	// apply_reference_wrapper
	//

	template< typename T > struct apply_reference_wrapper { using type = T; };
	template< typename T > struct apply_reference_wrapper< std::reference_wrapper<T> > { using type = T&; };

	template< typename T >
	using apply_reference_wrapper_t = typename apply_reference_wrapper<T>::type;

	//
	// identity
	//

	template< typename T > struct identity { using type = T; };

	template< typename T >
	using identity_t = typename identity<T>::type;

	template< typename Dst > Dst implicit_cast(typename identity<Dst>::type t) { return t; }

} // namespace luabind

#endif // LUABIND_TYPETRAITS_HPP_INCLUDED

