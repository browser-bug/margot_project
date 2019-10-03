/* core/goal.hpp
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

#include <algorithm>
#include <cassert>
#include <memory>

#include "margot/enums.hpp"
#include "margot/monitor.hpp"

namespace margot {

namespace helper {

/**
 * @brief helper struct, to compare two numbers accorfing to the comparison function
 *
 * @tparam cf The target comparison function (e.g. grater than or less than)
 *
 * @details
 * This struct provides a global interface that compares two number, using the correct
 * operator, given the target comparison function. For example it uses the < operator
 * for the comparison function "greater than".
 * This struct takes advantage of partial specialization to select the correct operator.
 * Since this struct represents the general case, you should never be able to use this
 * struct.
 */
template <ComparisonFunctions cf>
struct goal;

/**
 * @brief Specialization of the helper struct, for the "grater" comparison function
 *
 * @see helper::goal
 */
template <>
struct goal<ComparisonFunctions::GREATER> {
  /**
   * @brief Test wether value is grater than goal
   *
   * @tparam T The type of the value parameter
   * @tparam Y The type of the goal parameter
   *
   * @param [in] value The actual value that we have to compare
   * @param [in] goal The value of the goal to achieve
   *
   * @return True, if value is grater than goal
   */
  template <class T, class Y>
  inline static bool compare(const T value, const Y goal) {
    using promotion_type = decltype(T{} + Y{});
    return static_cast<promotion_type>(value) > static_cast<promotion_type>(goal);
  }
};

/**
 * @brief Specialization of the helper struct, for the "grater or equal " comparison function
 *
 * @see helper::goal
 */
template <>
struct goal<ComparisonFunctions::GREATER_OR_EQUAL> {
  /**
   * @brief Test wether value is grater or equal than goal
   *
   * @tparam T The type of the value parameter
   * @tparam Y The type of the goal parameter
   *
   * @param [in] value The actual value that we have to compare
   * @param [in] goal The value of the goal to achieve
   *
   * @return True, if value is grater or equal than goal
   */
  template <class T, class Y>
  inline static bool compare(const T value, const Y goal) {
    using promotion_type = decltype(T{} + Y{});
    return static_cast<promotion_type>(value) >= static_cast<promotion_type>(goal);
  }
};

/**
 * @brief Specialization of the helper struct, for the "less" comparison function
 *
 * @see helper::goal
 */
template <>
struct goal<ComparisonFunctions::LESS> {
  /**
   * @brief Test wether value is less than goal
   *
   * @tparam T The type of the value parameter
   * @tparam Y The type of the goal parameter
   *
   * @param [in] value The actual value that we have to compare
   * @param [in] goal The value of the goal to achieve
   *
   * @return True, if value is less than goal
   */
  template <class T, class Y>
  inline static bool compare(const T value, const Y goal) {
    using promotion_type = decltype(T{} + Y{});
    return static_cast<promotion_type>(value) < static_cast<promotion_type>(goal);
  }
};

/**
 * @brief Specialization of the helper struct, for the "less or equal" comparison function
 *
 * @see helper::goal
 */
template <>
struct goal<ComparisonFunctions::LESS_OR_EQUAL> {
  /**
   * @brief Test wether value is less or equal than goal
   *
   * @tparam T The type of the value parameter
   * @tparam Y The type of the goal parameter
   *
   * @param [in] value The actual value that we have to compare
   * @param [in] goal The value of the goal to achieve
   *
   * @return True, if value is less or equal than goal
   */
  template <class T, class Y>
  inline static bool compare(const T value, const Y goal) {
    using promotion_type = decltype(T{} + Y{});
    return static_cast<promotion_type>(value) <= static_cast<promotion_type>(goal);
  }
};

}  // namespace helper

/**
 * @brief This class represents a target to achieve
 *
 * @tparam T The type of the variable that stores the goal value
 * @tparam cf Enumerator that selects the target comparison function
 *
 * @details
 * This class defines a goal that must be achieved. It provides several methods
 * to check if a statistical property of the monitor have that property or if
 * a value achive the goal.
 * The most important feature of this class is that it stores the goal value as
 * a pointer (shared_ptr), therefore it is used to represents the numeric value
 * of a constraint.
 */
template <typename T, ComparisonFunctions cf>
class Goal {
 public:
  /**
   * @brief Aliasing of the type used to express the goal value
   */
  using value_type = T;

  /**
   * @brief A static reference to the comparison function
   */
  static constexpr ComparisonFunctions comparison_function = cf;

  /**
   * @brief Default constructor
   *
   * @param [in] goal_value The actual numeric value of the goal
   *
   * @details
   * To have a trivial constructor, the input paramter defaults to the default
   * value of the type T, ususally zero.
   */
  Goal(const T goal_value = T{}) : goal_value(new T{goal_value}) {}

  /**
   * @brief Set the value of the goal
   *
   * @param new_value The numeric value of the new goal value
   */
  inline void set(const T new_value) { *goal_value = new_value; }

  /**
   * @brief Retrieve the numeric value of the goal
   *
   * @return The numeric value of the goal
   */
  inline T get(void) const { return *goal_value; }

  /**
   * @brief Test wether the input parameter achieves the goal
   *
   * @tparam Y The type of the input parameters
   *
   * @param value The target value to evaluate
   *
   * @return True, if helper::goal<cf>::compare returns true
   */
  template <class Y>
  inline bool check(const Y value) const {
    assert(goal_value && "Error: checking an empty goal");
    return helper::goal<cf>::compare(value, *goal_value);
  }

  /**
   * @brief Test wether the first input parameter achieves the goal of the second parameter
   *
   * @tparam Y The type of the first input parameter
   * @tparam Z The type of the second input parameter
   *
   * @param value1 The first input parameter, i.e. the value to be evaluated
   * @param value2 The second input parameter, i.e. the goal value used to evaluate value1
   *
   * @return True, if helper::goal<cf>::compare(value1,value2) returns true
   */
  template <class Y, class Z>
  inline bool check(const Y value1, const Z value2) const {
    assert(goal_value && "Error: checking an empty goal");
    return helper::goal<cf>::compare(value1, value2);
  }

  /**
   * @brief Test wether a monitor' statistical properties achieve the goal
   *
   * @tparam Y The type of elements stored in the monitor
   * @tparam K The statistical type used to compute statistical properties of the monitor
   * @tparam df The statistical property of interest
   *
   * @param monitor The monitor that must be evaluated
   *
   * @return True, if helper::goal<cf>::compare returns true
   *
   * @details
   * If the statistical property extracted from the monitor is not valid, the goal
   * is considered not achieved.
   */
  template <class Y, class K, DataFunctions df>
  inline bool check(const Monitor<Y, K>& monitor) const {
    assert(goal_value && "Error: checking an empty goal");

    // get the statistical provider from the monitor
    auto buffer = monitor.get_buffer();

    // get the statistical information from the buffer
    bool is_valid = false;
    auto value = monitor_utils<Y, df>::get(buffer, is_valid);

    // return the comparison value (it it is valid)
    return is_valid && helper::goal<cf>::compare(value, *goal_value);
  }

  /**
   * @brief Compute the relative error of the input parameter, wrt the goal value
   *
   * @tparam Y The type of the input parameter
   *
   * @param[in] value The numeric value to evaluate
   *
   * @return The value of the relative error between the value and the goal
   *
   * @details
   * The error is always positive. If the goal is achieved, the error is zero.
   * Every time the goal value is zero, we have a numerical problem. Because, it will
   * trigger a division by zero exception. To solve this issue, we add constant
   * stride (with value 1), to both, the numerator and the denumerator. This
   * introduce a distorsion on the relative error.
   */
  template <class Y>
  inline auto relative_error(const Y value) const -> decltype(T{} + Y{} + float{}) {
    assert(goal_value && "Error: computing the relative error of an empty goal");

    using Z = decltype(T{} + Y{} + float{});

    if (helper::goal<cf>::compare(value, *goal_value)) {
      // the goal is respected, the error is zero
      return static_cast<Z>(0);
    } else {
      // avoid the division by zero pitfall by adding a fixed stride
      const bool safe_division = *goal_value != 0;
      const Z numerator = safe_division ? static_cast<Z>(value) : static_cast<Z>(value) + static_cast<Z>(1);
      const Z denumerator =
          safe_division ? static_cast<Z>(*goal_value) : static_cast<Z>(*goal_value) + static_cast<Z>(1);

      // compute the relative error (zero based)
      return std::abs((numerator / denumerator) - static_cast<Z>(1));
    }
  }

  /**
   * @brief Compute the relative error of a statistical property of the monitor, wrt the goal value
   *
   * @tparam Y The type of elements stored in the monitor
   * @tparam K The statistical type used to compute statistical properties of the monitor
   * @tparam df The statistical property of interest
   *
   * @param monitor The target monitor to evaluate
   *
   * @return The value of the relative error between the statistical property and the goal
   *
   * @details
   * The error is always positive. If the goal is achieved, the error is zero.
   * Every time the goal value is zero, we have a numerical problem. Because, it will
   * trigger a division by zero exception. To solve this issue, we add constant
   * stride (with value 1), to both, the numerator and the denumerator. This
   * introduce a distorsion on the relative error.
   */
  template <class Y, class K, DataFunctions df>
  inline auto relative_error(const Monitor<Y, K>& monitor) const -> decltype(T{} + Y{} + float{}) {
    assert(goal_value && "Error: computing the relative error of an empty goal");

    // get the statistical provider from the monitor
    auto buffer = monitor.get_buffer();

    // get the statistical information from the buffer
    bool is_valid = false;
    auto value = monitor_utils<Y, df>::get(buffer, is_valid);

    // check if it is ok to have a relative error
    return relative_error<Y>(value);
  }

  /**
   * @brief Compute the absolute error of the input parameter, wrt the goal value
   *
   * @tparam Y The type of the input parameter
   *
   * @param value The numeric value to evaluate
   *
   * @return The value of the absolute error between the value and the goal
   *
   * @details
   * The error is always positive. If the goal is achieved, the error is zero.
   */
  template <class Y>
  inline auto absolute_error(const Y value) const -> decltype(T{} + Y{} + float{}) {
    assert(goal_value && "Error: computing the absolute error of an empty goal");

    using Z = decltype(T{} + Y{} + float{});

    if (helper::goal<cf>::compare(value, *goal_value)) {
      // the goal is respected, the error is zero
      return static_cast<Z>(0);
    } else {
      // we must promote the type of the operands at least to float value
      const Z min_value = std::min(static_cast<Z>(value), static_cast<Z>(*goal_value));
      const Z max_value = std::max(static_cast<Z>(value), static_cast<Z>(*goal_value));

      // compute the relative error (zero based)
      return max_value - min_value;
    }
  }

  /**
   * @brief Compute the absolute error of a statistical property of the monitor, wrt the goal value
   *
   * @tparam Y The type of elements stored in the monitor
   * @tparam K The statistical type used to compute statistical properties of the monitor
   * @tparam df The statistical property of interest
   *
   * @param monitor The target monitor to evaluate
   *
   * @return The value of the absolute error between the statistical property and the goal
   *
   * @details
   * The error is always positive. If the goal is achieved, the error is zero.
   */
  template <class Y, class K, DataFunctions df>
  inline auto absolute_error(const Monitor<Y, K>& monitor) const -> decltype(T{} + Y{} + float{}) {
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
  /**
   * @brief A shared pointer to the goal value
   */
  std::shared_ptr<T> goal_value;
};

}  // namespace margot

#endif  // MARGOT_GOAL_HDR
