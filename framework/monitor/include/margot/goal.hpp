/* core/monitor
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


#ifndef MARGOT_GOAL_HEADER
#define MARGOT_GOAL_HEADER



#include <functional>
#include <memory>
#include <stdexcept>
#include <string>




#include "margot/config.hpp"
#include "margot/monitor.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	 * @brief The available comparison function
	 *
	 * @details
	 * This enumeration represents all the comparison functions
	 * used by the framework.
	 * It is meant to be used to express the comparison in a more
	 * human-friendly way
	 */
	enum class ComparisonFunction : uint_fast8_t
	{
		Greater = 0,
		GreaterOrEqual,
		Less,
		LessOrEqual
	};




	/**
	 * @brief This class represents a Goal
	 *
	 * @details
	 * This class is meant to represent an application requirement
	 * over both, an observed metric and a metric known only at design-time.
	 * Each Goal target one DataFunction of a monitor and perform only one
	 * comparison.
	 * This means that if you want to express an interval of admissible values,
	 * you should use two different Goals.
	 */
	class goal_t
	{

		public:


			/****************************************************
			 * Constructor and Deconstructors
			 ****************************************************/

			/**
				* @brief default constructor
			**/
			goal_t( void );

			/**
			 * @brief Constructor from a Monitor
			 *
			 * @param monitor The target monitor
			 *
			 * @param d_fun The DataFunction object of the goal
			 *
			 * @param c_fun The ComparisonFunction object of the goal
			 *
			 * @param goal_value The actual value of the goal
			 *
			 * @details
			 * This construnctor is meant to be used to define a goal over a metric that it is
			 * observed a runtime.
			 */
			template<typename T>
			goal_t(monitor_t<T>& monitor, const DataFunction& d_fun, const ComparisonFunction& c_fun, const statistical_properties_t& goal_value): goal_t(c_fun, goal_value)
			{

				// get a reference from the monitor
				const auto monitor_ref = monitor.get_monitor_concept();

				// assign it to the base class
				data->monitor_ptr = monitor_ref;

				// construct the lambda function
				switch (d_fun)
				{
					case (DataFunction::Average):
						data->observed_value = [ monitor_ref ] (statistical_properties_t& result)
						{
							return monitor_ref->get_average(result);
						};

						break;

					case (DataFunction::Variance):
						data->observed_value = [ monitor_ref ] (statistical_properties_t& result)
						{
							return monitor_ref->get_variance(result);
						};

						break;

					case (DataFunction::Max):
						data->observed_value = [ monitor_ref ] (statistical_properties_t& result)
						{
							return monitor_ref->get_max(result);
						};

						break;

					case (DataFunction::Min):
						data->observed_value = [ monitor_ref ] (statistical_properties_t& result)
						{
							return monitor_ref->get_min(result);
						};

						break;

					default:
						throw std::logic_error("Goal Error: Unable to understand the DataFunction ");
				}
			}


			/**
			 * @brief Constructor from an external function
			 *
			 * @param target_function The function used to retrieve the observed value
			 *
			 * @param c_fun The ComparisonFunction object of the goal
			 *
			 * @param goal_value The actual value of the goal
			 *
			 * @details
			 * This function is meant to be used to define a goal where the observed value are obtained by an external function,
			 * typically this constructor is used by the AS-RTM to create a goal on a metric not observed a runtime
			 */
			goal_t(std::function<bool(statistical_properties_t&)> target_function,  const ComparisonFunction& c_fun, const statistical_properties_t& goal_value);


			/**
			 * @brief Copy constructor
			 */
			goal_t(const goal_t& source) = default;

			/**
			 * @brief Move constructor
			 */
			goal_t( goal_t&& source ) = default;

			/**
			 * @brief Assign operator ( Copy semantic )
			 */
			goal_t& operator=( const goal_t& source ) = default;

			/**
			 * @brief Assign operator ( Move semantic )
			 */
			goal_t& operator=( goal_t&& source ) = default;

			/**
			 * @brief Deconstructor
			 */
			~goal_t( void ) = default;







			/****************************************************
			 * Class methods
			 ****************************************************/

			/**
			 * @brief Update the new target value.
			 *
			 * @param new_value The new target value of the Goal.
			 *
			 * @details
			 * The previous goal value will be discarded.
			 */
			template<typename T>
			inline void set(const T& new_value)
			{
				data->value = static_cast<statistical_properties_t>(new_value);
			}

			/**
			 * @brief Retrieve the goal value
			 *
			 * @return the actual goal value
			 */
			inline statistical_properties_t get( void )
			{
				return data->value;
			}

			/**
			 * @brief Get the observed value
			 *
			 * @return The value used to perform the comparison
			 *
			 * @details
			 * This attribute stands for the function used to retrieve the
			 * value observed by a monitor or an external function
			 */
			inline bool observed_value(statistical_properties_t& result)
			{
				return data->observed_value(result);
			}

			/**
			 * @brief Clear the observation window of a monitor (if any)
			 */
			inline void clear( void )
			{
				data->clear();
			}


			/**
			 * @brief Perform the intended comparison
			 *
			 * @param value1 The first value
			 *
			 * @param value2 The second value
			 *
			 * @return True if value1 is ComparisonFunction than value2
			 *
			 * @details
			 * This function is used to "remember" the comparison function of the Goal.
			 * For instance if it is chosen ComparisonFunction::Greater then this funtion
			 * is equivalent to:
			 *  { return value1 > value2 }
			 */
			inline bool compare(const statistical_properties_t& lhs, const statistical_properties_t& rhs)
			{
				return data->compare(lhs, rhs);
			}

			/**
			 * @brief Check if the goal has been reached
			 *
			 * @return True, if the goal is reached
			 *
			 * @details
			 * If the observed measure is not valid, than the goal is considered reached
			 */
			bool check( void );

			/**
			 * @brief Get the relative error
			 *
			 * @return The computed relative error
			 *
			 * @details
			 * The relative error is computed even if the circular buffer of the monitor is not full
			 */
			statistical_properties_t relative_error( void );


			/**
			 * @brief Get the absolute error
			 *
			 * @return The computed absolute error
			 *
			 * @details
			 * The absolute error is computed even if the circular buffer of the monitor is not full
			 */
			statistical_properties_t absolute_error( void );


			/**
			 * @brief Get the Normalized Actual Penalty
			 *
			 * @return The computed Normalized Actual Penalty
			 *
			 * @details
			 * The Normalized Actual Penalty (NAP) is computed even if the circular buffer of the monitor is not full
			 * The NAP is defined as:
			 *   abs(goal_value - observed_value) / (goal_value + observed_value)
			 */
			statistical_properties_t nap( void );



			/****************************************************
			 * Monitor internal representation
			 ****************************************************/

			/**
			 * @brief The internal data srtucture
			*
			* @details
			* A goal should the following informations:
			*   - a reference to the target monitor, if any
			*   - the value of a goal
			*   - the function that provides the observed value, if any
			*   - the function used to compare two values
			*/
			struct target_t
			{
				/**
				 * @brief A reference to the monitor
				 *
				 * @details
				 * If the metric is not been observed by a monitor at runtime
				 * this attribute is a nullpointer
				 */
				monitor_concept_ptr_t monitor_ptr;


				/**
				 * @brief Clear the observation window of a monitor (if any)
				 */
				inline void clear( void )
				{
					if (monitor_ptr)
					{
						monitor_ptr->clear();
					}
				}


				/**
				 * @brief The target value of the goal
				 */
				statistical_properties_t value;


				/**
				 * @brief Get the observed value
				 *
				 * @return The value used to perform the comparison
				 *
				 * @details
				 * This attribute stands for the function used to retrieve the
				 * value observed by a monitor or an external function
				 */
				std::function<bool(statistical_properties_t&)> observed_value;

				/**
				 * @brief Perform the intended comparison
				 *
				 * @param value1 The first value
				 *
				 * @param value2 The second value
				 *
				 * @return True if value1 is ComparisonFunction than value2
				 *
				 * @details
				 * This function is used to "remember" the comparison function of the Goal.
				 * For instance if it is chosen ComparisonFunction::Greater then this funtion
				 * is equivalent to:
				 *  { return value1 > value2 }
				 */
				std::function<bool(const statistical_properties_t&, const statistical_properties_t&)> compare;
			};


			/**
			 * @brief Typedef for the pointer to the internal structure
			 */
			using target_t_ptr = std::shared_ptr<struct target_t>;


			/**
			 * @brief Get a target pointer reference
			 *
			 * @return A smart pointer to the internal buffer
			 */
			inline target_t_ptr get_target( void ) const
			{
				return data;
			}


		private:

			/**
			 * @brief Default constructor
			 *
			 * @details
			 * This constructor is used to finalize all the other functions and
			 * it must be used only after the other constructors have initialized
			 * the getter function
			 */
			goal_t(const ComparisonFunction& c_fun, const statistical_properties_t& value);



			/**
			 * @ brief the refence to the internal structure
			 */
			target_t_ptr data;
	};


}



#endif // MARGOT_GOAL_HEADER
