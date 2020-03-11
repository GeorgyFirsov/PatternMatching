#pragma once

#include <type_traits>
#include <tuple>
#include <utility>

#include "ArgsOf.h"
#include "Traits.h"


/************************************************************************************
 * Match function
 * 
 * The key-concept is following:
 *  - We go through each (worst case) functor passed as parameter.
 *  - On each step we compare current functor's parameters with passed to 'Match' arguments.
 *  - In case of equality we make an attempt to call functor with specified parameters.
 *  - Otherwise we continue looking for matched functor.
 *  - The last passed functor can be a generic lambda - default functor, that matches
 *    to any number and any types of arguments.
 *    
 ************************************************************************************/

namespace match {
namespace detail {

    /************************************************************************************/

    //
    // Forward declarations
    // 
    
    template<typename _TiedArgs, typename _Callable, typename... _Callables>
    decltype(auto) _Match_Impl( _TiedArgs&&, _Callable&&, _Callables&&... );

    template<typename _TiedArgs, typename _Callable> 
    decltype(auto) _Match_Impl( _TiedArgs&&, _Callable&& );

    /************************************************************************************/

    //
    // Function template used to make an attempt
    // to invoke functor with arguments passed
    // from user code as tuple.
    // 

    template<
        typename  _TiedArgs /* Type of tuple of arguments */,
        typename  _Callable /* Type of callable, that matches arguments */,
        size_t... _Idxs     /* Indices of arguments in tuple */
    > decltype(auto) _Match_Impl_Call(
        _TiedArgs&& tpl      /* Tuple of arguments */,
        _Callable&& callable /* Function or functional object to call */,
        std::index_sequence<_Idxs...> /* Sequence to deduce indices */
    )
    {
        //
        // Check if we have correct number of indices
        // 
        static_assert(
            std::tuple_size<_TiedArgs>::value == sizeof...( _Idxs ),
            "Tuple size and corresponding indices mismatch."
        );

        //
        // Check if callable can be invoked with passed arguments
        // 
        static_assert(
            traits::is_invocable<_Callable, decltype( std::get<_Idxs>( tpl ) )... >::value,
            "Callable should be able to be invoked with passed arguments. "     \
            "Maybe you forgot to add trailing generic lambda to 'Match' call " \
            "or it has incorrect number of parameters."
        );

        //
        // Here we just invoke our function with untied
        // and forwarded arguments.
        //         
        return std::forward<_Callable>( callable )( 
            std::get<_Idxs>( std::forward<_TiedArgs>( tpl ) )... 
        );
    }

    /************************************************************************************/

    //
    // Overload for matched function (used in tag dispatch).
    // Here we make an attempt to call the functor.
    // 

    template<
        typename    _TiedArgs  /* Type of tuple with arguments */,
        typename    _Callable  /* Type of callable to try to call on this step */,
        typename... _Callables /* Types of rest callables (necessary only for unified interface) */
    > decltype(auto) _Match_Impl_Dispatch(
        std::true_type /* dummy */,
        _TiedArgs&& tpl /* Arguments to pass */,
        _Callable&& callable /* Callable to try to call */,
        _Callables&&... /* dummy */
    )
    {
        //
        // Forward all to caller function, that
        // will unpack arguments and make an attempt
        // to invoke callee.
        // 
        return _Match_Impl_Call( 
            std::forward<_TiedArgs>( tpl ),
            std::forward<_Callable>( callable ),
            std::make_index_sequence<std::tuple_size<_TiedArgs>::value>{}
        );
    }

    //
    // This overload is a "failure" overload and used in case
    // of arguments and parameters types mismatch.
    // 
    
    template<
        typename    _TiedArgs  /* Type of tuple with arguments */,
        typename    _Callable  /* Type of callable to try to call on this step */,
        typename... _Callables /* Types of rest callables (necessary only for unified interface) */
    > decltype(auto) _Match_Impl_Dispatch(
        std::false_type /* dummy */,
        _TiedArgs&& tpl /* Arguments to pass to callee */,
        _Callable&& /* dummy */,
        _Callables&&... callables /* Rest candidates to be called */
    )
    {
        //
        // Go one step backwards, but with rest of callables
        // 
        return _Match_Impl(
            std::forward<_TiedArgs>( tpl ),
            std::forward<_Callables>( callables )...
        );
    }

    /************************************************************************************/

    //
    // Function template, that used to compare types of arguments
    // and parameters for each functor until success or end of 
    // callables' list.
    // 

    template<
        typename    _TiedArgs  /* Type of tuple with arguments */,
        typename    _Callable  /* Type of current callable */,
        typename... _Callables /* Rest callables' types */
    > decltype(auto) _Match_Impl(
        _TiedArgs&& tpl /* Tuple with arguments */,
        _Callable&& callable /* Functor to try to call */,
        _Callables&&... callables /* Rest (untested for now) functors */
    )
    {
        //
        // First of all we need to check if types of arguments
        // for calling a functor are the same with passed to
        // match::Match as first parameter.
        // 
        using _ExactArgs = utils::ArgsOf<_Callable>;
        using _MatchCondition = traits::disjunction<
            traits::is_same_tuple_weak<_ExactArgs::Args,  _TiedArgs>,
            traits::is_same_tuple_weak<_ExactArgs::LRefs, _TiedArgs>,
            traits::is_same_tuple_weak<_ExactArgs::RRefs, _TiedArgs>
        >;

        //
        // _MatchCondition is inherited from one of
        // std::true_type or std::false_type, so we can use
        // tag dispatch techinque.
        // 
        return _Match_Impl_Dispatch(
            _MatchCondition{},
            std::forward<_TiedArgs>( tpl ),
            std::forward<_Callable>( callable ),
            std::forward<_Callables>( callables )...
        );
    }

    template<
        typename _TiedArgs /* Type of tuple with arguments */,
        typename _Callable /* Type of last callable */
    > decltype(auto) _Match_Impl(
        _TiedArgs&& tpl /* Tuple with arguments */,
        _Callable&& callable /* Last remaining callable */
    )
    {
        //
        // Here we have only one callable left. If it isn't
        // invocable with our arguments, a compilation error
        // will be produced by 'static_assert'.
        // 
        return _Match_Impl_Call(
            std::forward<_TiedArgs>( tpl ),
            std::forward<_Callable>( callable ),
            std::make_index_sequence<std::tuple_size<_TiedArgs>::value>{}
        );
    }

    /************************************************************************************/

} // namespace detail

                             /* ^^^  Library internals  ^^^ */
    /************************************************************************************/
                             /* vvv       User API      vvv */

    template<
        typename    _TiedArgs  /* Type of std::tuple with arguments */,
        typename... _Callables /* Types of functors to try to invoke */
    > 
    /* We don't know a return type here, but we need 
     * to deduce it "as is" => we use decltype(auto).
     * The same situation with funtions in 'details' ns. */
    decltype(auto) Match( 
        _TiedArgs&& tpl /* std::tuple with args. May contain references */, 
        _Callables&&... callables /* Variadic pack of callables */
    )
    {
        //
        // Check if we have at least one callable
        // 
        static_assert( 
            sizeof...( _Callables ) > 0,
            "At least one callable required in match::Match."
        );

        return detail::_Match_Impl(
            std::forward<_TiedArgs>( tpl ), std::forward<_Callables>( callables )...
        );
    }


#   define DEFAULT_FUNCTOR_DEF []( auto&&... )
#   define DEFAULT_THROW( _ExceptionT ) []( auto&&... ) { throw _ExceptionT(); }
#   define DEFAULT_THROW_MSG( _ExceptionT, _Msg ) []( auto&&... ) { throw _ExceptionT( _Msg ); }
#   define DEFAULT_RETURN_NULL []( auto&&... ) { return nullptr; }
#   define DEFAULT_RETURN_VALUE( _Value ) []( auto&&... ) { return (_Value); } 

} // namespace match
