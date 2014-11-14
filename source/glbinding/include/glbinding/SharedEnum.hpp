#pragma once

#include <glbinding/SharedEnum.h>

namespace glbinding 
{

template <typename T>
SharedEnumBase<T>::SharedEnumBase(T value)
: m_value(value)
{
}

template <typename T>
SharedEnumBase<T>::operator T() const
{
    return m_value;
}


template <typename Type>
SharedEnum<Type>::SharedEnum(Type value)
: SharedEnumBase<UnderlyingType>(static_cast<UnderlyingType>(value))
{
}

template <typename Type>
SharedEnum<Type>::SharedEnum(UnderlyingType value)
: SharedEnumBase<UnderlyingType>(value)
{
}

template <typename Type>
SharedEnum<Type>::operator Type() const
{
    return static_cast<Type>(this->m_value);
}

template <typename Type>
SharedEnum<Type> SharedEnum<Type>::operator+(UnderlyingType other)
{
    return static_cast<SharedEnum<Type>>(static_cast<UnderlyingType>(this->m_value) + other);
}



template <typename Type, typename... Types>
SharedEnum<Type, Types...>::SharedEnum(Type value)
: SharedEnum<Types...>(static_cast<UnderlyingType>(value))
{
}

template <typename Type, typename... Types>
SharedEnum<Type, Types...>::SharedEnum(UnderlyingType value)
: SharedEnum<Types...>(value)
{
}

template <typename Type, typename... Types>
SharedEnum<Type, Types...>::operator Type() const
{
    return static_cast<Type>(this->m_value);
}

template <typename Type, typename... Types>
SharedEnum<Type, Types...> SharedEnum<Type, Types...>::operator+(UnderlyingType other) const
{
    return static_cast<SharedEnum<Type, Types...>>(static_cast<UnderlyingType>(this->m_value) + other);
}

} // namespace glbinding
