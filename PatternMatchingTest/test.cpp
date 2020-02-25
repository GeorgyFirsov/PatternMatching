#include "pch.h"

TEST(PatternMatching, MatchValues)
{
    auto result = match::Match(
        std::make_tuple( 1 ),
        []( double )       { return 0; },
        []( int )          { return 1; },
        []( char )         { return 2; },
        []( std::string& ) { return 3; },
        DEFAULT_FUNCTOR    { return 4; }
    );

    EXPECT_EQ( result, 1 );
}

TEST(PatternMatching, MatchLRefs)
{
    int test = 0;

    match::Match(
        std::tie( test ),
        []( double& t )      { t = 0; },
        []( int& t )         { t = 1; },
        []( char& t )        { t = 2; },
        []( std::string& t ) { t = 3; },
        /* You can specify your own default functor */
        []( auto&& t )       { t = 4; }
    );

    EXPECT_EQ( test, 1 );
}

TEST(PatternMatching, MatchRRefs)
{
    std::string str = "Non empty";

    EXPECT_FALSE( str.empty() );

    auto copy = match::Match(
        std::forward_as_tuple( str ),
        []( std::string& s ) { auto copy = std::move( s ); return copy; },
        DEFAULT_FUNCTOR      { return std::string( "Error" ); }
    );

    EXPECT_TRUE( str.empty() );
    EXPECT_FALSE( copy.empty() );

    EXPECT_EQ( copy, std::string( "Non empty" ) );
}