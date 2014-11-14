#pragma once

#include <type_traits>

namespace glbinding 
{

template <typename... Types>
class SharedEnum;

// intersection

template<typename T, typename... Types>
struct is_member_of_SharedEnum
{
    static const bool value = false;
};

template<typename T, typename U, typename... Types>
struct is_member_of_SharedEnum<T, U, Types...>
{
    static const bool value = std::conditional<std::is_same<T, U>::value, std::true_type, is_member_of_SharedEnum<T, Types...>>::type::value;
};

template<typename, typename>
struct prepend_to_SharedEnum
{};

template<typename T, typename... Types>
struct prepend_to_SharedEnum<T, SharedEnum<Types...>>
{
    using type = SharedEnum<T, Types...>;
};

template<typename, typename>
struct intersect_SharedEnum
{
    using type = SharedEnum<>;
};

template<typename T, typename... Types, typename... OtherTypes>
struct intersect_SharedEnum<SharedEnum<T, Types...>, SharedEnum<OtherTypes...>>
{
    using type = typename std::conditional<!is_member_of_SharedEnum<T, OtherTypes...>::value, typename intersect_SharedEnum<SharedEnum<Types...>, SharedEnum<OtherTypes...>>::type, typename prepend_to_SharedEnum<T, typename intersect_SharedEnum<SharedEnum<Types...>, SharedEnum<OtherTypes...>>::type>::type>::type;
};

// implementation

template <typename T>
class SharedEnumBase
{
    using UnderlyingType = T;

public:
    SharedEnumBase(T value);

    explicit operator T() const;
protected:
    T m_value;
};

template <>
class SharedEnum<>
{};

template <typename Type>
class SharedEnum<Type> : public SharedEnumBase<typename std::underlying_type<Type>::type>
{
public:
    SharedEnum(Type value);
    SharedEnum(UnderlyingType value);

    operator Type() const;

    SharedEnum<Type> operator+(UnderlyingType other) const;
};

template <typename Type, typename... Types>
class SharedEnum<Type, Types...> : public SharedEnum<Types...>
{
public:
    SharedEnum(Type value);
    SharedEnum(UnderlyingType value);

    operator Type() const;

    SharedEnum<Type, Types...> operator+(UnderlyingType other) const;
};

} // namespace glbinding

#include <glbinding/SharedEnum.hpp>
