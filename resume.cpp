//
// Written with GNU Emacs 25.2.2, 26.1
//
// On Debian GNU/Linux 10, with G++ 8.3.0 please compile using:
// `make CXX="g++-8" CXXFLAGS="-std=c++2a -Wall -Wextra -Werror" resume`
//
// You can just `make CXXFLAGS="-Wall -Wextra -Werror resume` if you have a
// more recent C++20 enabled compiler.
//
// Compiled with GCC 8.3.0 on Debian GNU/Linux 10.
//
// Copyright 2019, Stéphane ALBERT
//

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public
// License along with this program.  If not, see
// <https://www.gnu.org/licenses/>.
//

#include <ctime>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>


/*******************************************************************************/
//
// We define positions as enum-class values in order to benefit from
// enum-class scope and robustness of not being implicitely cast to
// int (such as a old-style enum).
//
enum class position : int
{
  none = -1,
  //
  babylon_software,
  cs_vr,
  diginext,
  cs_space,
  geown,
  //
  count,
  first = none,
  last = geown,
};

/*******************************************************************************/
//
// We define a traits class in order to statically associate data
// (begin year, company, etc.) to positions.
//
template< position P >
struct position_traits
{
  // Get position value (in case of local type-definition).
  static constexpr position value() noexcept { return P; }

  // Remain undefined here in order to generate compile-time
  // error/warning if reached.
  //
  // So, if a new position is added, the compiler will generate output
  // if related traits are not implemented.
  // {
  static constexpr char const * name() noexcept;

  static constexpr std::size_t year() noexcept;
  // }
};


/*******************************************************************************/
//
// Factorize position traits specilization using macro definition.
//
#define DECLARE_POSITION_TRAITS( Year, Position, Name )             \
  template<>                                                        \
  struct position_traits< Position >                                \
  {                                                                 \
    static constexpr char const * name() noexcept { return Name; }  \
                                                                    \
    static constexpr std::size_t year() noexcept { return Year; }   \
  }

//
// Implement position traits specializations.
//
DECLARE_POSITION_TRAITS( 2020, position::geown, "Geown (MD Group)" );
DECLARE_POSITION_TRAITS( 2012, position::cs_space, "CS, Space Dpt" );
DECLARE_POSITION_TRAITS( 2010, position::diginext, "Diginext (CS Group);" );
DECLARE_POSITION_TRAITS( 2003, position::cs_vr, "CS, Virtual-Reality Dpt" );
DECLARE_POSITION_TRAITS( 2001, position::babylon_software, "Babylon Software" );

/*******************************************************************************/
//
// Factorize traits helper function declaration using macro
// definition.
//
#define DECLARE_HELPER( trait )                 \
  template< position P >                        \
  constexpr                                     \
  auto                                          \
  trait() noexcept                              \
  {                                             \
    return position_traits< P >::trait();       \
  }

// Implement traits helper functions.
DECLARE_HELPER( name )
DECLARE_HELPER( year )

/*******************************************************************************/
namespace details
{
  //
  // Factorize application of unary-operator to position enum-class.
  //
  template< position P,
            typename UnaryOp >
  constexpr
  position
  eval( UnaryOp unary_op ) noexcept
  {
    using underlying_type_t = std::underlying_type_t< position >;

    return
      static_cast< position >(
        unary_op(
          static_cast< underlying_type_t >( P )
          )
        );
  }
} // namespace details.

/*******************************************************************************/
//
// Next position.
//
template< position P >
constexpr
position
next() noexcept
{
  return details::eval< P >(
    []( auto p ) { return p + 1; }
    );
}

/*******************************************************************************/
//
// Previous position.
//
template< position P >
constexpr
position
prev() noexcept
{
  return details::eval< P >(
    []( auto p ) { return p - 1; }
    );
}

/*******************************************************************************/
//
// Number of years between two positions.
//
template< position LHS, position RHS >
constexpr
std::size_t
diff() noexcept
{
  static_assert( ::year< LHS >() >= ::year< RHS >() );

  return ::year< LHS >() - ::year< RHS >();
}

/*******************************************************************************/
//
// Begin "static iterator".
//
constexpr
position
begin() noexcept
{
  return next< position::none >();
}

/*******************************************************************************/
//
// End "static iterator".
//
constexpr
position
end() noexcept
{
  return position::count;
}

/*******************************************************************************/
//
// First position.
//
constexpr
position
first() noexcept
{
  return begin();
}

/*******************************************************************************/
//
// Last (or current) position.
//
constexpr
position
last() noexcept
{
  return prev< end() >();
}

/*******************************************************************************/
//
// Factorize output of year.
//
template< position P >
constexpr
void
print_end_year()
{
  std::cout << year< next< P >() >();
}


/*******************************************************************************/
//
// Specialize output of year for current position.
//
template<>
void
print_end_year< last() >()
{
  std::cout << std::setw( 4 ) << ' ';
}

/*******************************************************************************/
//
// Display position line.
//
template< position P >
constexpr
std::size_t
print()
{
  std::cout << year< P >() << "-";

  print_end_year< P >();

  std::cout << ": " << name< P >() << std::endl;

  return year< P >();
}

/*******************************************************************************/
//
// 1.Print position in resume and recurse to previous one
// (anti-chronoligc order).
//
// Accumulate years as a parameter in order to ensure terminal
// recursion (instead of simply returning it).
//
// Return sum of years when recursion has ended.
//
// 2. Specialize resursion in order to terminate when first position is
// reached.
//
// 3. Helper function in order to ease call of recursion template meta-program.
//
#define DEFINE_MAPPER( mapper, accum, fun )               \
  namespace details                                       \
  {                                                       \
    template< position P >                                \
      constexpr                                           \
      auto                                                \
      mapper( accum a )                                   \
    {                                                     \
      fun< P >();                                         \
                                                          \
      constexpr auto before = prev< P >();                \
                                                          \
      return mapper< before >( a + diff< P, before >() ); \
    }                                                     \
                                                          \
    template<>                                            \
      constexpr                                           \
      auto                                                \
      mapper< begin() >( accum a )                        \
    {                                                     \
      fun< begin() >();                                   \
                                                          \
      return a;                                           \
    }                                                     \
  }                                                       \
                                                          \
  constexpr                                               \
  auto                                                    \
  mapper( accum a )                                       \
  {                                                       \
    return details::mapper< last() >( a );                \
  }                                                       \

DEFINE_MAPPER( print_resume, std::size_t, ::print )

/*******************************************************************************/
//
// Get system local-time and extract current year.
//
int
current_year()
{
  std::time_t sys_now = std::time( nullptr );
  std::tm * local_now = std::localtime( &sys_now );

  if( !local_now )
    throw std::runtime_error( "Failed to retrieve local time." );

  return local_now->tm_year + 1900;
}

/*******************************************************************************/
int
main( int, char * [] )
{
  try
  {
    std::cout
      << print_resume(  current_year() - ::year< last() >() )
      << " years of professional C++ :)" << std::endl;
  }
  catch( std::exception & e )
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
