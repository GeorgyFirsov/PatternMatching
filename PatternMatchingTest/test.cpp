#include "pch.h"

TEST(PatternMatching, MatchValues)
{
    auto result = match::Match(
        std::make_tuple( 1 ),
        []( double )       { return 0; },
        []( int )          { return 1; },
        []( char )         { return 2; },
        []( std::string& ) { return 3; },
        DEFAULT_RETURN_VALUE( 4 )
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
        DEFAULT_RETURN_VALUE( std::string( "Error" ) )
    );

    EXPECT_TRUE( str.empty() );
    EXPECT_FALSE( copy.empty() );

    EXPECT_EQ( copy, std::string( "Non empty" ) );
}

TEST(PatternMatching, MatchValuesMultiple)
{
    auto result = match::Match(
        std::make_tuple( 1, 3.14 ),
        []( double )         { return 0; },
        []( int, double )    { return 1; },
        []( char, int, int ) { return 2; },
        []( std::string& )   { return 3; },
        DEFAULT_RETURN_VALUE( 4 )
    );

    EXPECT_EQ( result, 1 );
}

TEST(PatternMatching, MatchCompliletimeCondition)
{
    auto result = match::Match(
        std::make_tuple( std::is_floating_point<float>{} ),
        []( std::true_type )  { return true; },
        []( std::false_type ) { return false; }
        /* Note: here is not necessary to put default functor */
    );

    EXPECT_TRUE( result );
}

TEST(PatternMatching, MatchCompliletimeConditionMultiple)
{
    auto result = match::Match(
        std::make_tuple( std::is_floating_point<float>{}, std::is_array<float>{} ),
        []( std::false_type )                 { return false; },
        []( std::true_type, std::false_type ) { return true; },
        DEFAULT_RETURN_VALUE( false )
    );

    EXPECT_TRUE( result );
}

bool DoWorkInt( int )         { return false; }
bool DoWorkDbl( double )      { return true; }
bool DoWorkIntInt( int, int ) { return false; }

TEST(PatternMatching, MatchFunctions)
{
    auto result = match::Match(
        std::make_tuple( 3.14 ),
        DoWorkInt,
        DoWorkIntInt,
        DoWorkDbl
    );

    EXPECT_TRUE( result );
}