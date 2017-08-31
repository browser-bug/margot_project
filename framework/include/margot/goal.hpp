/* core/rank.hpp
 * Copyright (C) 2017 Davide Gadioli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef MARGOT_GOAL_HDR
#define MARGOT_GOAL_HDR


#include <memory>
#include <algorithm>
#include <cassert>


#include "margot/enums.hpp"
#include "margot/monitor.hpp"


namespace margot
{

  namespace helper
  {
    template< ComparisonFunctions cf >
    struct goal;

    template< >
    struct goal< ComparisonFunctions::GREATER >
    {
      template< class T, class Y >
      inline static bool compare( const T value, const Y goal )
      {
        using promotion_type = decltype(T{} + Y{});
        return static_cast<promotion_type>(value) > static_cast<promotion_type>(goal);
      }
    };

    template< >
    struct goal< ComparisonFunctions::GREATER_OR_EQUAL >
    {
      template< class T, class Y >
      inline static bool compare( const T value, const Y goal )
      {
        using promotion_type = decltype(T{} + Y{});
        return static_cast<promotion_type>(value) >= static_cast<promotion_type>(goal);
      }
    };

    template< >
    struct goal< ComparisonFunctions::LESS >
    {
      template< class T, class Y >
      inline static bool compare( const T value, const Y goal )
      {
        using promotion_type = decltype(T{} + Y{});
        return static_cast<promotion_type>(value) < static_cast<promotion_type>(goal);
      }
    };

    template< >
    struct goal< ComparisonFunctions::LESS_OR_EQUAL >
    {
      template< class T, class Y >
      inline static bool compare( const T value, const Y goal )
      {
        using promotion_type = decltype(T{} + Y{});
        return static_cast<promotion_type>(value) <= static_cast<promotion_type>(goal);
      }
    };
  }


  template< typename T, ComparisonFunctions cf >
  class Goal
  {

    public:

      using value_type = T;

      static constexpr ComparisonFunctions comparison_function = cf;

      Goal( void ): goal_value(nullptr)
      {}

      Goal( const T goal_value ): goal_value(new T{goal_value})
      {}


      inline void set( const T new_value )
      {
        assert(goal_value && "Error: setting a value to an empty goal");
        *goal_value = new_value;
      }

      inline T get( void ) const
      {
        assert(goal_value && "Error: getting the value of an empty goal");
        return *goal_value;
      }


      template< class Y >
      inline bool check( const Y value ) const
      {
        assert(goal_value && "Error: checking an empty goal");
        return helper::goal<cf>::compare(value, *goal_value);
      }

      template< class Y, class Z >
      inline bool check( const Y value1, const Z value2 ) const
      {
        assert(goal_value && "Error: checking an empty goal");
        return helper::goal<cf>::compare(value1, value2);
      }

      template< class Y, DataFunctions df >
      inline bool check( const Monitor<Y>& monitor ) const
      {
        assert(goal_value && "Error: checking an empty goal");

        // get the statistical provider from the monitor
        auto buffer = monitor.get_buffer();

        // get the statistical information from the buffer
        bool is_valid = false;
        auto value = monitor_utils<Y, df>::get(buffer, is_valid);

        // return the comparison value (it it is valid)
        return is_valid && helper::goal<cf>::compare(value, *goal_value);
      }


      template< class Y >
      inline auto relative_error( const Y value ) const -> decltype(T {} + Y{} + float{})
      {
        assert(goal_value && "Error: computing the relative error of an empty goal");

        using Z = decltype(T{} + Y{} + float{});

        if ( helper::goal<cf>::compare(value, *goal_value) )
        {
          // the goal is respected, the error is zero
          return static_cast<Z>(0);
        }
        else
        {
          // avoid the division by zero pitfall by adding a fixed stride
          const bool safe_division = *goal_value != 0;
          const Z numerator = safe_division ? static_cast<Z>(value) : static_cast<Z>(value) + static_cast<Z>(1);
          const Z denumerator = safe_division ? static_cast<Z>(*goal_value) : static_cast<Z>(*goal_value) + static_cast<Z>(1);

          // compute the relative error (zero based)
          return std::abs((numerator / denumerator) - static_cast<Z>(1));
        }
      }

      template< class Y, DataFunctions df >
      inline auto relative_error( const Monitor<Y>& monitor ) const -> decltype(T {} + Y{} + float{})
      {
        assert(goal_value && "Error: computing the relative error of an empty goal");

        // get the statistical provider from the monitor
        auto buffer = monitor.get_buffer();

        // get the statistical information from the buffer
        bool is_valid = false;
        auto value = monitor_utils<Y, df>::get(buffer, is_valid);

        // check if it is ok to have a relative error
        return relative_error<Y>(value);
      }



      template< class Y >
      inline auto absolute_error( const Y value ) const -> decltype(T {} + Y{} + float{})
      {
        assert(goal_value && "Error: computing the absolute error of an empty goal");

        using Z = decltype(T{} + Y{} + float{});

        if ( helper::goal<cf>::compare(value, *goal_value) )
        {
          // the goal is respected, the error is zero
          return static_cast<Z>(0);
        }
        else
        {
          // we must promote the type of the operands at least to float value
          const Z min_value = std::min( static_cast<Z>(value), static_cast<Z>(*goal_value));
          const Z max_value = std::max( static_cast<Z>(value), static_cast<Z>(*goal_value));

          // compute the relative error (zero based)
          return max_value - min_value;
        }
      }

      template< class Y, DataFunctions df >
      inline auto absolute_error( const Monitor<Y>& monitor ) const -> decltype(T {} + Y{} + float{})
      {
        assert(goal_value && "Error: computing the absolute error of an empty goal");

        // get the statistical provider from the monitor
        auto buffer = monitor.get_buffer();

        // get the statistical information from the buffer
        bool is_valid = false;
        auto value = monitor_utils<Y, df>::get(buffer, is_valid);

        // check if it is ok to have a relative error
        return absolute_error<Y>(value);
      }


    private:

      std::shared_ptr<T> goal_value;

  };






}

#endif // MARGOT_GOAL_HDR
