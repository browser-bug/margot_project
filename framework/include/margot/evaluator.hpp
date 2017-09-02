/* core/evaluator
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


#ifndef MARGOT_EVALUATOR_HDR
#define MARGOT_EVALUATOR_HDR

#include <cstddef>
#include <functional>
#include <cmath>
#include <memory>

#include "margot/operating_point.hpp"
#include "margot/traits.hpp"
#include "margot/enums.hpp"
#include "margot/knowledge_base.hpp"

namespace margot
{

  /**
   * @brief All the information required to extract a value from an Operating Point
   *
   * @tparam segment The target segment of the Operating Point
   * @tparam bound The value of the target bound (lower or upper)
   * @tparam field The index of the target field within the segment of the Operating Point
   * @tparam sigma_std The number of times we would like to consider the standard deviation (if any)
   * @tparam T The type of the coefficient
   *
   * @details
   * This class is meant to identify which is the value of interest that needs to be extracted from the
   * Operating Point.
   * To extract the average value, it is enaugh to set to zero the sigma_std template parameter. In this
   * case the value of the bound parameter is meaningless.
   * Since the user might want to weight the extracted value, it is possible to specify a parameter to alter
   * this value.
   *
   * @see Evaluator
   */
  template< OperatingPointSegments segment, BoundType bound, std::size_t field, int sigma_std, typename T = float >
  struct OPField
  {


    /**
     * @brief Default constructor
     *
     * @param [in] coefficient The coefficient used to weight the measure
     *
     * @details
     * Since the weight of the measure it is not always meaningfull, we default
     * it to 1.
     */
    OPField( const T coefficient = 1): coefficient(coefficient) {}


    /**
     * @brief Explicit definition of the type of the coefficient
     */
    using value_type = T;


    /**
     * @brief The enumerator that identify the target segment
     */
    static constexpr OperatingPointSegments target_segment = segment;


    /**
     * @brief The enumerator that identify the bound of interest
     */
    static constexpr BoundType target_bound = bound;


    /**
     * @brief The index of target field within the target segment
     */
    static constexpr std::size_t field_index = field;


    /**
     * @brief The number of times that we want to consider the standard deviation (if any)
     */
    static constexpr int sigma = sigma_std;


    /**
     * @brief The actual coefficient used to weught the measure
     */
    const T coefficient;
  };


  /**
   * @brief This class evaluates an Operating Point.
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam FieldComposer How to compose the value computed from the Operating Point
   * @tparam ...Fields Variadic list that identify the fields of interest
   *
   * @see OPField
   *
   * @details
   * This class is the generic evaluator of an Operating Point, it uses partial specialization
   * to define the correct way of evaluating an Operating Point.
   */
  template< class OperatingPoint, FieldComposer composer, class ...Fields>
  class Evaluator;


  /******************************************************************
   *  SPECIALIZATION OF THE EVALUATOR FOR SINGLE FIELD
   ******************************************************************/


  /**
   * @brief A simple evaluation of a field of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam Field The target field of the Operating Point
   *
   * @see OPField, Evaluator
   *
   * @details
   * This class represents the most simple way to evaluate an Operating Point, in
   * particular it extracts the target value (identified by the Field type) from a
   * single field of the Operating Point.
   * For this reason, this implementation does not take into account the value of
   * the coefficient specified in the Field type, nor its type.
   */
  template< class OperatingPoint, class Field>
  class Evaluator< OperatingPoint, FieldComposer::SIMPLE, Field >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the evaluator handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Explicit definition to the helper struct that evaluates the Operating Point
       */
      using value_extractor = op_utils<OperatingPoint, Field::target_segment, Field::target_bound, Field::field_index, Field::sigma>;


      /**
       * @brief Explicit definition of the type of the evaluation of the Operating Point
       *
       * @details
       * Since this struct extracts direcly a value from the Operating Point, the type
       * of the evaluation value is the type of the extracted field.
       */
      using value_type = typename value_extractor::value_type;


      /**
       * @brief Evaluates the target Operating Point
       *
       * @param [in] target_op A shared pointer to the target Operating Point
       *
       * @return The value of the target field extracted from the Operating Point
       */
      inline static value_type evaluate(const OperatingPointPtr& target_op, const Field = 1)
      {
        return value_extractor::get(target_op);
      }

  };


  /******************************************************************
   *  SPECIALIZATION OF THE EVALUATOR FOR LINEAR COMPOSITION
   ******************************************************************/


  /**
   * @brief Evaluate a linear combination of fields of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam Field The current evaluated field
   * @tparam ...Fields The remainder of the fields of interest
   *
   * @see OPField, Evaluator
   *
   * @details
   * This class represents a linear combination of fields of the Operating Point.
   * In particular it evaluates an Operating Point as:
   *   coef_1 * field_1 + coef_2 * field_2 + ... + coef_n * field_n
   * where coef_i is the coefficient of the i-th field and field_i represents the
   * value extracted from the i-th field.
   *
   * This struct exploits compile time recursion to compose the actual function
   * that evauluates the Operating Point.
   */
  template< class OperatingPoint, class Field, class ...Fields>
  class Evaluator< OperatingPoint, FieldComposer::LINEAR, Field, Fields... >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the evaluator handles object with is_operating_point trait");

    public:


      /**
       * @brief Explicit defintion of a pointer to the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;


      /**
       * @brief Explicit definition of the helper struct used to extract the value of the current field
       */
      using value_extractor = op_utils<OperatingPoint, Field::target_segment, Field::target_bound, Field::field_index, Field::sigma>;


      /**
       * @brief Explicit definition of the Evaluator used to compute the remainder of the fields
       */
      using next_bit_extractor = Evaluator< OperatingPoint, FieldComposer::LINEAR, Fields... >;


      /**
       * @brief The type of the value extracted by Evaluator of the remainder of the fields
       */
      using next_bit_type = typename next_bit_extractor::value_type;


      /**
       * @brief The type of the value extracted by the current field
       */
      using this_bit_type = typename value_extractor::value_type;


      /**
       * @brief The type of the coefficient of the current field
       */
      using this_bit_coef_type = typename Field::value_type;


      /**
       * @brief Explicit definition of the type of the evaluated value, up to the current field
       *
       * @details
       * This definition is useful to promote the type of the result according to the largest information
       * evaluated up to the current field, to prevent loss of information in the final computation.
       */
      using value_type = decltype( this_bit_type{} * next_bit_type{} * this_bit_coef_type{});


      /**
       * @brief Evaluates the target Operating Point
       *
       * @param [in] target_op A shared pointer to the target Operating Point
       * @param [in] field The current evaluated field (for the coefficient)
       * @param [in] ...other The remainder of the fields to be evaluated
       *
       * @return The value of the Operating Point, up to the current field
       *
       * @details
       * This methods exploits compile time recursion to compose the forumula that computes the final value
       * for the target Operating Point.
       */
      inline static value_type evaluate(const OperatingPointPtr& target_op, const Field field, const Fields... other )
      {
        return (value_extractor::get(target_op) * field.coefficient) + next_bit_extractor::evaluate(target_op, other...);
      }

  };


  /**
   * @brief Evaluate a linear combination of fields of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam Field The current (and last) field evaluated
   *
   * @see OPField, Evaluator
   *
   * @details
   * This class represents a linear combination of fields of the Operating Point.
   * In particular it evaluates an Operating Point as:
   *   coef_1 * field_1 + coef_2 * field_2 + ... + coef_n * field_n
   * where coef_i is the coefficient of the i-th field and field_i represents the
   * value extracted from the i-th field.
   *
   * This struct exploits compile time recursion to compose the actual function
   * that evauluates the Operating Point. This partial specialization represents
   * the last step in the recursion.
   */
  template< class OperatingPoint, class Field >
  class Evaluator< OperatingPoint, FieldComposer::LINEAR, Field >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the evaluator handles object with is_operating_point trait");

    public:


      /**
       * @brief Explicit defintion of a pointer to the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;


      /**
       * @brief Explicit definition of the helper struct used to extract the value of the current field
       */
      using value_extractor = op_utils<OperatingPoint, Field::target_segment, Field::target_bound, Field::field_index, Field::sigma>;


      /**
       * @brief The type of the value extracted by the current field
       */
      using this_bit_type = typename value_extractor::value_type;


      /**
       * @brief The type of the coefficient of the current field
       */
      using this_bit_coef_type = typename Field::value_type;


      /**
       * @brief Explicit definition of the type of the evaluated value.
       *
       * @details
       * This definition is useful to promote the type of the result according to the largest information
       * evaluated up to the current field, to prevent loss of information in the final computation. In the
       * context of this struct, it represents the last bit of information, used to compute all the other ones.
       */
      using value_type = decltype( this_bit_type{} * this_bit_coef_type{});


      /**
       * @brief Evaluates the target Operating Point
       *
       * @param [in] target_op A shared pointer to the target Operating Point
       * @param [in] field The current evaluated field (for the coefficient)
       *
       * @return The value of the Operating Point, in the final step
       *
       * @details
       * This methods exploits compile time recursion to compose the forumula that computes the final value
       * for the target Operating Point. In particular, this function compute the last bit of the final value.
       */
      inline static value_type evaluate(const OperatingPointPtr& target_op, const Field field )
      {
        return value_extractor::get(target_op) * field.coefficient;
      }

  };


  /******************************************************************
   *  SPECIALIZATION OF THE EVALUATOR FOR GEOMETRIC COMPOSITION
   ******************************************************************/


  /**
   * @brief Evaluate a geometric combination of fields of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam Field The current evaluated field
   * @tparam ...Fields The remainder of the fields of interest
   *
   * @see OPField, Evaluator
   *
   * @details
   * This class represents a geometric combination of fields of the Operating Point.
   * In particular it evaluates an Operating Point as:
   *    (field_1^coef_1) * (field_2^coef_2) * ... * (field_n^coef_n)
   * where coef_i is the coefficient of the i-th field and field_i represents the
   * value extracted from the i-th field.
   *
   * This struct exploits compile time recursion to compose the actual function
   * that evauluates the Operating Point.
   */
  template< class OperatingPoint, class Field, class ...Fields>
  class Evaluator< OperatingPoint, FieldComposer::GEOMETRIC, Field, Fields... >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the evaluator handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit defintion of a pointer to the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;


      /**
       * @brief Explicit definition of the helper struct used to extract the value of the current field
       */
      using value_extractor = op_utils<OperatingPoint, Field::target_segment, Field::target_bound, Field::field_index, Field::sigma>;


      /**
       * @brief Explicit definition of the Evaluator used to compute the remainder of the fields
       */
      using next_bit_extractor = Evaluator< OperatingPoint, FieldComposer::GEOMETRIC, Fields... >;


      /**
       * @brief The type of the value extracted by Evaluator of the remainder of the fields
       */
      using next_bit_type = typename next_bit_extractor::value_type;


      /**
       * @brief The type of the value extracted by the current field
       */
      using this_bit_type = typename value_extractor::value_type;


      /**
       * @brief The type of the coefficient of the current field
       */
      using this_bit_coef_type = typename Field::value_type;


      /**
       * @brief Explicit definition of the type of the evaluated value, up to the current field
       *
       * @details
       * This definition is useful to promote the type of the result according to the largest information
       * evaluated up to the current field, to prevent loss of information in the final computation.
       * This type must be at least a float value.
       */
      using value_type = decltype( this_bit_type{} * next_bit_type{} * this_bit_coef_type{} * float{});


      /**
       * @brief Evaluates the target Operating Point
       *
       * @param [in] target_op A shared pointer to the target Operating Point
       * @param [in] field The current evaluated field (for the coefficient)
       * @param [in] ...other The remainder of the fields to be evaluated
       *
       * @return The value of the Operating Point, up to the current field
       *
       * @details
       * This methods exploits compile time recursion to compose the forumula that computes the final value
       * for the target Operating Point.
       */
      inline static value_type evaluate(const OperatingPointPtr& target_op, const Field field, const Fields... other )
      {
        return std::pow(value_extractor::get(target_op), field.coefficient) * next_bit_extractor::evaluate(target_op, other...);
      }

  };


  /**
   * @brief Evaluate a geometric combination of fields of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam Field The current (and last) field evaluated
   *
   * @see OPField, Evaluator
   *
   * @details
   * This class represents a geometric combination of fields of the Operating Point.
   * In particular it evaluates an Operating Point as:
   *   (field_1^coef_1) * (field_2^coef_2) * ... * (field_n^coef_n)
   * where coef_i is the coefficient of the i-th field and field_i represents the
   * value extracted from the i-th field.
   *
   * This struct exploits compile time recursion to compose the actual function
   * that evauluates the Operating Point. This partial specialization represents
   * the last step in the recursion.
   */
  template< class OperatingPoint, class Field >
  class Evaluator< OperatingPoint, FieldComposer::GEOMETRIC, Field >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the evaluator handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit defintion of a pointer to the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;


      /**
       * @brief Explicit definition of the helper struct used to extract the value of the current field
       */
      using value_extractor = op_utils<OperatingPoint, Field::target_segment, Field::target_bound, Field::field_index, Field::sigma>;


      /**
       * @brief The type of the value extracted by the current field
       */
      using this_bit_type = typename value_extractor::value_type;


      /**
       * @brief The type of the coefficient of the current field
       */
      using this_bit_coef_type = typename Field::value_type;


      /**
       * @brief Explicit definition of the type of the evaluated value.
       *
       * @details
       * This definition is useful to promote the type of the result according to the largest information
       * evaluated up to the current field, to prevent loss of information in the final computation. In the
       * context of this struct, it represents the last bit of information, used to compute all the other ones.
       * This type must be at least a float value.
       */
      using value_type = decltype( this_bit_type{} * this_bit_coef_type{} * float{});


      /**
       * @brief Evaluates the target Operating Point
       *
       * @param [in] target_op A shared pointer to the target Operating Point
       * @param [in] field The current evaluated field (for the coefficient)
       *
       * @return The value of the Operating Point, in the final step
       *
       * @details
       * This methods exploits compile time recursion to compose the forumula that computes the final value
       * for the target Operating Point. In particular, this function compute the last bit of the final value.
       */
      inline static value_type evaluate(const OperatingPointPtr& target_op, const Field field )
      {
        return std::pow(value_extractor::get(target_op), field.coefficient);
      }

  };


}


#endif // MARGOT_EVALUATOR_HDR
