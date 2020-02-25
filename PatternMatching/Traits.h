#pragma once

namespace traits {

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

} // namespace traits