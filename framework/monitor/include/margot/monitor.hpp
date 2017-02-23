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


#ifndef MARGOT_MONITOR_HEADER
#define MARGOT_MONITOR_HEADER


#include <numeric>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <memory>

#include "margot/config.hpp"

/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	 * @brief The available statistical properties
	 *
	 * @details
	 * This enumerattion represents all the statistical properties that
	 * are computed by a monitor on the observed data
	 */
	enum class DataFunction : uint_fast8_t
	{
		Average = 0,
		Variance,
		Max,
		Min
	};






	/****************************************************
	 * The high level concept of a monitor
	 ****************************************************/

	/**
	 * @brief Common interface of a monitor
	 *
	 * @details
	 * This struct define a common interface that hides the implementation
	 * details of a generic monitor.
	 */
	struct monitor_concept_t
	{
		public:

			/**
			 * @brief Compute the variance
			 *
			 * @param [out] result The computed variance
			 *
			 * @return True, if the result is valid
			 *
			 * @details
			 * It actually computes the variance only once, unless the
			 * circular buffer is changed.
			 * A result is valid iff it is computed when the circular buffer is full.
			 */
			virtual  bool get_variance(margot::statistical_properties_t& result) = 0;

			/**
			 * @brief Compute the average
			 *
			 * @param [out] result The computed average
			 *
			 * @return True, if the result is valid
			 *
			 * @details
			 * It actually computes the average only once, unless the
			 * circular buffer is changed.
			 * A result is valid iff it is computed when the circular buffer is full.
			 */
			virtual bool get_average(margot::statistical_properties_t& result) = 0;

			/**
			 * @brief Find the maximum
			 *
			 * @param [out] result The maximum observed value
			 *
			 * @return True, if the result is valid
			 *
			 * @details
			 * It actually finds the minimum element only once, unless the
			 * circular buffer is changed.
			 * The result is valid iff it is computed when the circular buffer is full.
			 */
			virtual bool get_max(margot::statistical_properties_t& result) = 0;

			/**
			 * @brief Find the maximum
			 *
			 * @param [out] result The maximum observed value
			 *
			 * @return True, if the result is valid
			 *
			 * @details
			 * It actually finds the minimum element only once, unless the
			 * circular buffer is changed.
			 * The result is valid iff it is computed when the circular buffer is full.
			 */
			virtual bool get_min(margot::statistical_properties_t& result) = 0;

			/**
			 * @brief Erase all the observed values
			 */
			virtual void clear( void ) = 0;


			/**
			 * @brief Check whether the observation window is full
			 *
			 * @return True, if the observation window is full
			 */
			virtual bool full( void ) const = 0;


			/**
			 * @brief Check whether the observation window is empty
			 *
			 * @return True, if the observation window is empty
			 */
			virtual bool empty( void ) const = 0;


			virtual ~monitor_concept_t( void ) = default;
	};

	/**
	 * @brief Typedef to the pointer of a circular buffer
	 */
	using monitor_concept_ptr_t = std::shared_ptr<monitor_concept_t>;


	/**
	 * @brief The base monitor object
	 *
	 * @details
	 * This class represents a monitor that implements all the functionalities
	 * required by the concept of a monitor.
	 * This class is a manager of the observed data, tipically
	 * obtained by a derivate class that actually implements the methods
	 * that gather the measure.
	 * This class defines the following functionalities:
	 *   - store the last observed data in a observation window (circular buffer)
	 *   - manage the the circular buffer
	 *   - extracts statistical properties over the observated data
	 *
	 * @note
	 * This class is basically a wrapper for the real monitor. In this way is more
	 * efficient copy/move/assign a monitor in the source code
	 */
	template< typename T >
	class monitor_t
	{
		public:

			/**
			 * @brief Typedef to the value type
			 */
			using value_type = T;

			/****************************************************
			 * Constructors and destructor of the monitor
			 ****************************************************/

			/**
			 * @brief Default constructor of a monitor
			 *
			 * @param [in] max_size The maximum number of elements stored in a monitor
			 *
			 * @param [in] min_size The minum number of elements to consider for a valid measure
			 *
			 * @details
			 * The maximum size should be greater than zero, otherwise is thrown a logic_error
			 */
			monitor_t(const std::size_t max_size = 1, const std::size_t min_size = 1)
			{
				buffer.reset(new circular_buffer_t(max_size, min_size));
			}

			/**
			 * @brief The copy constructor
			 *
			 * @param [in] source The other monitor that we should copy from
			 *
			 * @details
			 * Since we cannot use a template in a standard copy constructor, we are not able
			 * to duplicate the internal buffer.
			 * But we are able to move the buffer to a new monitor, which is the most logical operation.
			 */
			monitor_t(const monitor_t& source) = default;

			/**
			 * @brief Move constructor
			 *
			 * @param [in] source The source monitor
			 *
			 * @details
			 * It actually move the circular buffer pointer, thus the old monitor has a nullpointer and
			 * it should not be used anymore
			 */
			monitor_t(monitor_t&& source) = default;

			/**
			 * @brief Assign operator (move semantic)
			 *
			 * @param [in] source The source monitor
			 *
			 * @details
			 * The assign operation will move a monitor to another
			 */
			monitor_t& operator=(monitor_t&& source) = default;

			/**
			 * @brief Assign operator (copy semantic)
			 *
			 * @param [in] source The source monitor
			 */
			monitor_t& operator=(const monitor_t& source) = default;

			/**
			 * @brief The default deconstructor
			 */
			~monitor_t() = default;










			/****************************************************
			 * The public interface of the monitor
			 ****************************************************/


			/**
			 * @brief Insert a new value in the circular buffer
			 *
			 * @details
			 * This methods insert a new observed value in the circular buffer,
			 * it also takes in account to wrap the index of the the next element
			 * according to the buffer size.
			 */
			inline void push(const T& new_value)
			{
				buffer->push(new_value);
			}

			/**
			 * @brief Retrieve the last element observed
			 *
			 * @return The last observed data
			 */
			inline T last() const
			{
				return buffer->last();
			}


			/**
			 * @brief Retrieve the size of the circular buffer
			 *
			 * @return The actual size of the circular buffer
			 */
			inline std::size_t size( void ) const
			{
				return buffer->size();
			}


			/**
			 * @brief Clear the circular buffer
			 *
			 * @details
			 * It actually reduce the size of the circular buffer to zero and reserve the required
			 * space for new values.
			 */
			inline void clear( void )
			{
				buffer->clear();
			}

			/**
			 * @brief Check whether the observation window is full
			 *
			 * @return True, if the observation window is full
			 */
			inline bool full( void ) const
			{
				return buffer->full();
			}

			/**
			 * @brief Check whether the observation window is empty
			 *
			 * @return True, if the observation window is empty
			 */
			inline bool empty( void ) const
			{
				return buffer->empty();
			}

			/**
			 * @brief Compute the variance over the observed data
			 *
			 * @return The computed variance
			 *
			 * @details
			 * The return type depends on the type of elements stored in the
			 * circular buffer and the type used to compute the statistical properties
			 */
			inline auto variance( void ) -> decltype(statistical_properties_t{} / T{})
			{
				return buffer->variance();
			}

			/**
			 * @brief Compute the variance over the observed data
			 *
			 * @param [out] result The computed variance
			 *
			 * @return True, if the variance is being computed with at least one value
			 *
			 * @details
			 * The type of the result depends on the type of elements stored in the
			 * circular buffer and the type used to compute the statistical properties
			 */
			inline bool variance(decltype(statistical_properties_t{} / T{})& result)
			{
				return buffer->variance(result);
			}

			/**
			 * @brief Compute the average over the observed data
			 *
			 * @return The computed average
			 *
			 * @details
			 * The return type depends on the type of elements stored in the
			 * circular buffer and the type used to compute the statistical properties
			 */
			inline auto average( void ) -> decltype(statistical_properties_t{} / T{})
			{
				return buffer->average();
			}

			/**
			 * @brief Compute the average over the observed data
			 *
			 * @param [out] result The computed average
			 *
			 * @return True, if the average is being computed with at least one value
			 *
			 * @details
			 * The type of the result depends on the type of elements stored in the
			 * circular buffer and the type used to compute the statistical properties
			 */
			inline bool average(decltype(statistical_properties_t{} / T{})& result)
			{
				return buffer->average(result);
			}

			/**
			 * @brief Find the maximum element over the observed data
			 *
			 * @return The maximum element
			 */
			inline T max( void )
			{
				return buffer->max();
			}

			/**
			 * @brief Find the maximum element over the observed data
			 *
			 * @param [out] result The maximum element
			 *
			 * @return True, if the maximum element is pfound with at least one value
			 */
			inline bool max(T& result)
			{
				return buffer->max(result);
			}

			/**
			 * @brief Find the minumum element over the observed data
			 *
			 * @return The minimum element
			 */
			inline T min( void )
			{
				return buffer->min();
			}

			/**
			 * @brief Find the minimum element over the observed data
			 *
			 * @param [out] result The minumum element
			 *
			 * @return True, if the minimum element is found with at least one value
			 */
			inline bool min(T& result)
			{
				return buffer->min(result);
			}

			/**
			 * @brief Return a reference to the monitor concept
			 *
			 * @return A shared pointer to a monitor_concept_t
			 *
			 * @details
			 * This method is used by a goal_t object to retrieve a reference to the target monitor.
			 * In this way a goal does not need further datails about the monitoring element
			 */
			inline monitor_concept_ptr_t get_monitor_concept( void ) const
			{
				return std::static_pointer_cast<monitor_concept_t>(buffer);
			}











		private:





			/****************************************************
			 * Monitor implementation
			 ****************************************************/

			/**
			 * @brief The monitor implementation
			 *
			 * @details
			 * This class implements the actual montor as a circular buffer that stores
			 * the observed values and implements the functions that computes
			 * the statistical properties
			 */
			struct circular_buffer_t: public monitor_concept_t
			{
					static_assert( std::is_arithmetic<T>::value, "A monitor requires an arithmetic-like type");

				public:

					// to handle the case where T has double precision, while statistical_properties_t is less precise
					using internal_average_t = decltype(statistical_properties_t{} / T{});


					/****************************************************
					 * Constructors and destructor of the internal model
					 ****************************************************/

					/**
					 * @brief The default constructor
					 */
					circular_buffer_t(const std::size_t max_size, const std::size_t min_size):
						max_size(max_size),
						min_size(min_size)
					{
						if (max_size == 0)
						{
							throw std::logic_error("Unable to create a monitor with maximum size equal to zero");
						}

						circular_buffer.reserve(max_size);
						next_element_index = 0;
						invalidate();
					}

					/**
					 * @brief The copy constructor
					 */
					circular_buffer_t(const circular_buffer_t& source) = default;

					/**
					 * @brief The assign operator
					 */
					circular_buffer_t& operator=(const circular_buffer_t&  source) = default;

					/**
					 * @brief The move constructor
					 */
					circular_buffer_t(circular_buffer_t&& source) = default;

					/**
					 * @brief The default destructor is fine
					 */
					~circular_buffer_t() = default;













					/****************************************************
					 * Methods that affect the circular buffer
					 ****************************************************/

					/**
					 * @brief Insert a new value in the circular buffer
					 *
					 * @details
					 * This methods insert a new observed value in the circular buffer,
					 * it also takes in account to wrap the index of the the next element
					 * according to the buffer size.
					 */
					void push(const T& new_value)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// insert the value
						if (circular_buffer.size() < max_size)
						{
							// handle the case on which the buffer is not fully allocated
							circular_buffer.emplace_back(new_value);
							++next_element_index;
						}
						else
						{
							// the buffer is fully allocated, mind the overflow
							if ( next_element_index == max_size)
							{
								circular_buffer[0] = new_value;
								next_element_index = 1;
							}
							else
							{
								circular_buffer[next_element_index] = new_value;
								++next_element_index;
							}
						}

						// invalidate the previous measures
						invalidate();
					}

					/**
					 * @brief Clear the circular buffer
					 */
					void clear( void )
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// erase all the stored elements
						circular_buffer.clear();
						circular_buffer.reserve(max_size);
						next_element_index = 0;

						// invalidate the previous measures
						invalidate();
					}

					/**
					 * @brief Retrieve the last element observed
					 *
					 * @return The last observed data
					 */
					T last() const
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);
						return circular_buffer[next_element_index - 1];
					}

					/**
					 * @brief Retrieve the size of the circular buffer
					 *
					 * @return The actual size of the circular buffer
					 */
					inline std::size_t size( void ) const
					{
						return circular_buffer.size();
					}


					/**
					 * @brief Check whether the observation window is full
					 *
					 * @return True, if the observation window is full
					 */
					inline bool full( void ) const
					{
						return circular_buffer.size() == max_size;
					}


					/**
					 * @brief Check whether the observation window is empty
					 *
					 * @return True, if the observation window is empty
					 */
					inline bool empty( void ) const
					{
						return circular_buffer.empty();
					}
















					/****************************************************
					 * Methods that wraps the computation of a Data Function
					 ****************************************************/

					internal_average_t variance( void )
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						compute_variance();
						return previous_variance;
					}
					bool variance(internal_average_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// compute it
						compute_variance();
						result = previous_variance;
						return  circular_buffer.size()  >= min_size;
					}
					bool get_variance(statistical_properties_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// compute it
						compute_variance();
						result = static_cast<statistical_properties_t>(previous_variance);
						return  circular_buffer.size()  >= min_size;
					}

					internal_average_t average( void )
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						compute_average();
						return previous_average;
					}
					bool average(internal_average_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						compute_average();
						result = previous_average;
						return  circular_buffer.size()  >= min_size;
					}
					bool get_average(statistical_properties_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						compute_average();
						result = static_cast<statistical_properties_t>(previous_average);
						return  circular_buffer.size()  >= min_size;
					}

					T max( void )
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						find_max();
						return previous_max;
					}
					bool max(T& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						find_max();
						result = previous_max;
						return circular_buffer.size()  >= min_size;
					}
					bool get_max(statistical_properties_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						// otherwise compute it
						find_max();
						result = static_cast<statistical_properties_t>(previous_max);
						return circular_buffer.size()  >= min_size;
					}

					T min( void )
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						find_min();
						return previous_min;
					}
					bool min(T& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						find_min();
						result = previous_min;
						return circular_buffer.size()  >= min_size;
					}
					bool get_min(statistical_properties_t& result)
					{
						// lock the buffer
						std::lock_guard<std::mutex> lg(lock);

						find_min();
						result = static_cast<statistical_properties_t>(previous_min);
						return circular_buffer.size() >= min_size;
					}












				private:


					/**
					 * @brief Invalidate the previous computed properties
					 */
					inline void invalidate( void )
					{
						already_computed[0] = false;
						already_computed[1] = false;
						already_computed[2] = false;
						already_computed[3] = false;
					}



					/****************************************************
					* Methods that actually compute the DataFunctions
					****************************************************/
					inline void compute_variance( void )
					{
						// check if the previous is ok
						if (already_computed[static_cast<std::size_t>(DataFunction::Variance)])
						{
							return;
						}

						// get the average
						compute_average();
						internal_average_t avg = previous_average;

						// compute the  variance
						internal_average_t sq_sum = static_cast<internal_average_t>(0);
						std::for_each(circular_buffer.cbegin(), circular_buffer.cend(), [&avg, &sq_sum] (const T & d)
						{
							internal_average_t diff = static_cast<internal_average_t>(d) - avg;
							sq_sum += diff * diff;
						});


						// find the best N according to the data
						const std::size_t coef = circular_buffer.size() > 1 ? circular_buffer.size() - 1 : 1;
						previous_variance = sq_sum / static_cast<internal_average_t>(coef);

						// update the flag & return
						already_computed[static_cast<std::size_t>(DataFunction::Variance)] = true;
					}

					inline void compute_average( void )
					{
						// check if the previous is ok
						if (already_computed[static_cast<std::size_t>(DataFunction::Average)])
						{
							return;
						}

						// compute the average
						previous_average = static_cast<internal_average_t>(std::accumulate(circular_buffer.cbegin(), circular_buffer.cend(), static_cast<internal_average_t>(0)));
						const internal_average_t size = static_cast<internal_average_t>(circular_buffer.size());
						previous_average /= size > 1 ? size : 1;

						// update the flag & return
						already_computed[static_cast<std::size_t>(DataFunction::Average)] = true;
					}

					inline void find_max( void )
					{
						// check if the previous is ok
						if (already_computed[static_cast<std::size_t>(DataFunction::Max)])
						{
							return;
						}

						// find the maximum
						const auto result = std::max_element(circular_buffer.cbegin(), circular_buffer.cend());
						previous_max = result != circular_buffer.cend() ? *result : 0;

						// update the flag & return
						already_computed[static_cast<std::size_t>(DataFunction::Max)] = true;
					}

					inline void find_min( void )
					{
						// check if the previous is ok
						if (already_computed[static_cast<std::size_t>(DataFunction::Min)])
						{
							return;
						}

						// find the minumum
						const auto result = std::min_element(circular_buffer.cbegin(), circular_buffer.cend());
						previous_min = result != circular_buffer.cend() ? *result : 0;

						// update the flag & return
						already_computed[static_cast<std::size_t>(DataFunction::Min)] = true;
					}








					/****************************************************
					 * Private members to handle the circular buffer
					 ****************************************************/

					/**
					 * @brief The actual container of the data
					 */
					std::vector<T> circular_buffer;

					/**
					 * @brief The maximum size of the array
					 */
					const std::size_t max_size;

					/**
					 * @brief The minumum size of the array to be evaluated
					 */
					const std::size_t min_size;

					/**
					 * @brief The reference to the next free element
					 */
					std::size_t next_element_index;








					/****************************************************
					 * Helper attributes
					 ****************************************************/

					/**
					 * @brief Optimization flag array
					 *
					 * @details
					 * Each element of this array states if we have already
					 * computed the required Data Function
					 */
					bool already_computed[4];

					/**
					 * @brief Store the previous average
					 */
					internal_average_t previous_average;

					/**
					 * @brief Store the previous variance
					 */
					internal_average_t previous_variance;

					/**
					 * @brief Store the previous maximum value
					 */
					T previous_max;

					/**
					 * @brief Store the previous minimum value
					 */
					T previous_min;

					/**
					 * @brief A lock to ensure thread safety
					 */
					mutable std::mutex lock;
			};


		protected:


			/****************************************************
			 * The pointer to the model (we are in monitor_t)
			 ****************************************************/

			/**
			 * @brief A reference to the used circular buffer
			 */
			std::shared_ptr< circular_buffer_t > buffer;
	};






} // namespace margot

#endif //MARGOT_MONITOR_HEADER
