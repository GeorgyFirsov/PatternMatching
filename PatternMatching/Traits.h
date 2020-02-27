#pragma once

namespace traits {

    /************************************************************************************/

    //
    // std::conjunction implementation
    // 

    template<typename... /* _BoolTs */> 
    struct conjunction : std::true_type { };

    template<typename _BoolT> 
    struct conjunction<_BoolT> : _BoolT { };

    template<typename _BoolT, typename... _BoolTs>
    struct conjunction<_BoolT, _BoolTs...>
        : std::conditional<bool( _BoolT::value ), conjunction<_BoolTs...>, _BoolT>::type { };

    /************************************************************************************/

    //
    // std::disjunction implementation
    // 

    template<typename... /* _Bools */>
    struct disjunction : std::false_type { };

    template<typename _BoolT>
    struct disjunction<_BoolT> : _BoolT { };

    template<typename _BoolT, typename... _BoolTs>
    struct disjunction<_BoolT, _BoolTs...>
        : std::conditional<bool(_BoolT::value), _BoolT, disjunction<_BoolTs...>>::type { };

    /************************************************************************************/

    //
    // std::is_invocable implementation.
    // It is NOT a full copy of std::invoke,
    // but it is enough for this library.
    // 

namespace {
    
    //
    // Helper template used for SFINAE in implementation of is_invokable.
    // It receives one and more arguments and invokes the first one
    // with rest as arguments.
    // 

    template<
        typename    _Callable /* Type of callable to invoke */, 
        typename... _Args     /* Types of arguments to pass */
    > decltype(auto) InvokeFirstWithRest( 
        _Callable&& callable /* Callable to invoke */, 
        _Args&&... args /* Arguments to pass into a functor */
    ) noexcept( 
        noexcept( std::forward<_Callable>( callable )( std::forward<_Args>( args )... ) )
    )
    {
        return std::forward<_Callable>( callable )( 
            std::forward<_Args>( args )... 
        );
    }

    //
    // Helper template, that exactly checks if callable can be invoked
    // with specified arguments
    // 

    template <typename _Void, typename... _Types>
    struct is_invocable_helper 
        : std::false_type { };

    template <typename... _Types>
    struct is_invocable_helper<
        std::void_t<decltype( InvokeFirstWithRest( std::declval<_Types>()... ) )>,
        _Types...
    > : std::true_type { };

} // anonymous namespace

    template <typename _Callable, typename... _Args>
    struct is_invocable 
        : is_invocable_helper<void, _Callable, _Args...> { };

    /************************************************************************************/

    //
    // Trait for checking if one tuple's elements are same with another tuple's
    // corresponding elements. Comparison is "weak", it means that we consider
    // as the same types that make a hierarchy.
    // 

namespace {

    //
    // Overload of helper function that ends recursive instantiation
    // 

    template<
        typename  _Tuple1 /* Type of the first tuple */,
        typename  _Tuple2 /* Type of the second tuple */
    > constexpr bool _IsSameTupleWeakHelper( std::index_sequence<> )
    {
        return true;
    }

    //
    // Overload that compares the most right unchecked element
    // 

    template<
        typename  _Tuple1 /* Type of the first tuple */,
        typename  _Tuple2 /* Type of the second tuple */,
        size_t    _Idx    /* Dummy index */,
        size_t... _Idxs   /* Indices, amount of which represent an index in tuple of current element */
    > constexpr bool _IsSameTupleWeakHelper( std::index_sequence<_Idx, _Idxs...> )
    {
        //
        // Aliases for cleaner code.
        // We extract here clean types of last unchecked 
        // corresponding elements.
        // 
        using _ElemType1 = typename std::decay<decltype( std::get<sizeof...( _Idxs )>( std::declval<_Tuple1>() ) )>::type;
        using _ElemType2 = typename std::decay<decltype( std::get<sizeof...( _Idxs )>( std::declval<_Tuple2>() ) )>::type;
        
        return \
            traits::disjunction<
                std::is_base_of<_ElemType1, _ElemType2>,
                std::is_base_of<_ElemType2, _ElemType1>,
                std::is_same   <_ElemType1, _ElemType2>
            >::value && _IsSameTupleWeakHelper<_Tuple1, _Tuple2>( std::make_index_sequence<sizeof...( _Idxs )>{} );
    }

    /************************************************************************************/

    //
    // Base type for 'is_same_tuple_weak' to inherit from.
    // It has two specializations for the same and for
    // different sizes of tuples.
    // 

    template<
        typename _Tuple1 /* Type of the first tuple */,
        typename _Tuple2 /* Type of the second one */,
        size_t   _Size1  /* Size of the first tuple */,
        size_t   _Size2  /* Size of the second one */
    > struct is_same_tuple_weak_base
        : std::false_type { };

    template<
        typename _Tuple1 /* Type of the first tuple */,
        typename _Tuple2 /* Type of the second one */,
        size_t   _Size   /* Size of tuples (specialization for same-sized tuples) */
    > struct is_same_tuple_weak_base<_Tuple1, _Tuple2, _Size, _Size>
        : std::bool_constant<
            /* For tuples with the same size we can compare corresponding types inside */
            _IsSameTupleWeakHelper<_Tuple1, _Tuple2>( std::make_index_sequence<_Size>{} )
        > { };

} // anonymous namespace

    template<
        typename _Tuple1 /* Type of the first tuple */,
        typename _Tuple2 /* Type of the second tuple */
    > struct is_same_tuple_weak
        : is_same_tuple_weak_base<
            _Tuple1, _Tuple2, 
            std::tuple_size<_Tuple1>::value, 
            std::tuple_size<_Tuple2>::value
        > { };

} // namespace traits
