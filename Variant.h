#ifndef VARIANT_H
#define VARIANT_H

#include <cassert>
#include <memory>
#include <typeinfo>

/*!
 *  \brief Type wrapper utility
 */
template <typename T>
struct TypeWrapper
{
	using TYPE = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	using CONSTTYPE = const TYPE;
	using REFTYPE = TYPE&;
	using CONSTREFTYPE = const TYPE&;
};

/*!
 *  \brief Simple variant class
 */
class Variant
{
public:
	Variant()
		: m_Impl{ nullptr }
	{
	}

	~Variant() = default;

	Variant( const Variant& other )
		: m_Impl{ !other.isEmpty() ? other.m_Impl->clone() : nullptr }
	{
	}

	Variant( Variant&& other ) noexcept
		: m_Impl{ std::move( other.m_Impl ) }
	{
	}

	Variant& operator=( const Variant& rhs )
	{
		if ( &rhs != this )
		{
			m_Impl = !rhs.isEmpty() ? rhs.m_Impl->clone() : nullptr;
		}
		return *this;
	}

	Variant& operator=( Variant&& rhs ) noexcept
	{
		if ( &rhs != this )
		{
			m_Impl = std::move( rhs.m_Impl );
		}
		return *this;
	}

	template<typename TargetType, typename T>
	static Variant fromValue( T&& inValue )
	{
		Variant result;
		result.m_Impl = std::make_unique<VariantImpl<typename TypeWrapper<TargetType>::TYPE>>( std::forward<T>( inValue ) );
		return result;
	}

	template<typename T>
	explicit Variant( T&& inValue )
		: m_Impl( new VariantImpl<typename TypeWrapper<T>::TYPE>{ std::forward<T>( inValue ) } )
	{
	}

	template<class T>
	typename TypeWrapper<T>::CONSTREFTYPE value() const
	{
		assert( typeid( *m_Impl ) == typeid( const VariantImpl<typename TypeWrapper<T>::TYPE> ) && "Type in Variant mismatches requested type" );
		return static_cast<const VariantImpl<typename TypeWrapper<T>::TYPE>*>( m_Impl.get() )->m_Value;
	}

	template<class T>
	void setValue( T&& inValue )
	{
		m_Impl.reset( new VariantImpl<typename TypeWrapper<T>::TYPE>{ std::forward<T>( inValue ) } );
	}

	bool isEmpty() const
	{
		return !m_Impl;
	}

private:
	struct AbstractVariantImpl
	{
		virtual ~AbstractVariantImpl() = default;
		virtual std::unique_ptr<AbstractVariantImpl> clone() = 0;
	};

	template<typename T>
	struct VariantImpl : public AbstractVariantImpl
	{
		template<typename U>
		explicit VariantImpl( U&& inValue )
			: m_Value( std::forward<U>( inValue ) )
		{
		}

		std::unique_ptr<AbstractVariantImpl> clone() override
		{
			return std::unique_ptr<AbstractVariantImpl>( new VariantImpl<T>{ m_Value } );
		}

		T m_Value;
	};

	std::unique_ptr<AbstractVariantImpl> m_Impl;
};

template<typename TargetType, typename... Args>
Variant make_variant( Args&&... args )
{
	return Variant{ TargetType{ std::forward<Args>( args )... } };
}

#endif // VARIANT_H
