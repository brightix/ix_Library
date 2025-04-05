#pragma once
#include <type_traits>

/* ���ڼ���Ƿ����ָ����Ա���� */
#define DEFINE_HAS_MEMBER_FUNC(FuncName) \
template<typename T> \
class Has_##FuncName{ \
private:\
	template<typename U>\
	static auto	test(int) -> decltype(std::declval<U>().FuncName(),std::true_type{});\
template<typename>\
static std::false_type test(...);	\
public:\
	static constexpr bool value = decltype(test<T>(0))::value;\
};

