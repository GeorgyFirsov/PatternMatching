#pragma once

#include <tuple>

namespace utils {
namespace {

    /************************************************************************************/

    //
    // Helper structure, that gives us some information about
    // callable (note: there's no callable passed as template
    // parameter to this structure), for instance types of
    // arguments.
    // 

    template<
        typename    _Ret  /* Return type of functor*/,
        typename... _Args /* Args types passed to functor */
    > struct ArgsOfBase
    {
        using Args  = std::tuple<_Args...>;   // tuple with exact types of passed arguments
        using LRefs = std::tuple<_Args&...>;  // tuple with lvalue references to passed arguments
        using RRefs = std::tuple<_Args&&...>; // tuple with rvalue references to passed arguments

        using Ret   = _Ret;
        using ArgsCount = std::integral_constant<size_t, sizeof...( _Args )>;

        constexpr size_t GetArgsCount() const noexcept {
            return ArgsCount::value;
        }
    };

} // anonymous namespace

    /************************************************************************************/

    //
    // User interface for using ArgsOfBase in code.
    // 

    template<
        typename _ClassType /* Type of functor */
    > struct ArgsOf 
      /* It's arguments are the same as in its operator() */
        : ArgsOf<decltype(&_ClassType::operator())> { };

    //
    // Pointer-to-function specialization
    // 

    template<
        typename    _Ret  /* Return value of function */,
        typename... _Args /* Arguments' types */
    > struct ArgsOf<_Ret(*)(_Args...)>
        : ArgsOfBase<_Ret, _Args...> { };

    //
    // Reference-to-unction specialization
    // 

    template<
        typename    _Ret  /* Return value of function */,
        typename... _Args /* Arguments' types */
    > struct ArgsOf<_Ret(&)(_Args...)>
        : ArgsOfBase<_Ret, _Args...> { };

    //
    // Template specializations for member functions
    // 

#define __PLACEHOLDER

#define __ARGS_OF_MEMFN( CONST_OPT, VOLATILE_OPT, REF_OPT )                 \
    template<typename _Ret, typename _Class, typename... _Args>             \
    struct ArgsOf<_Ret(_Class::*)(_Args...) CONST_OPT VOLATILE_OPT REF_OPT> \
        : ArgsOfBase<_Ret, _Args...> { }

    __ARGS_OF_MEMFN( __PLACEHOLDER, __PLACEHOLDER, __PLACEHOLDER );
    __ARGS_OF_MEMFN( __PLACEHOLDER, __PLACEHOLDER, &             );
    __ARGS_OF_MEMFN( __PLACEHOLDER, __PLACEHOLDER, &&            );
    __ARGS_OF_MEMFN( const,         __PLACEHOLDER, __PLACEHOLDER );
    __ARGS_OF_MEMFN( const,         __PLACEHOLDER, &             );
    __ARGS_OF_MEMFN( const,         __PLACEHOLDER, &&            );
    __ARGS_OF_MEMFN( __PLACEHOLDER, volatile,      __PLACEHOLDER );
    __ARGS_OF_MEMFN( __PLACEHOLDER, volatile,      &             );
    __ARGS_OF_MEMFN( __PLACEHOLDER, volatile,      &&            );
    __ARGS_OF_MEMFN( const,         volatile,      __PLACEHOLDER );
    __ARGS_OF_MEMFN( const,         volatile,      &             );
    __ARGS_OF_MEMFN( const,         volatile,      &&            );

#undef __PLACEHOLDER
#undef __ARGS_OF_MEMFN

} // namespace utils
