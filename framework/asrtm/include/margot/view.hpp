/* core/asrtm
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


#ifndef MARGOT_ASRTM_VIEW_HEADER
#define MARGOT_ASRTM_VIEW_HEADER


#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>
#include <memory>


#include "margot/config.hpp"
#include "margot/operating_point.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	* @brief  A view on a field of the Operating Point
	*
	* @details
	* This class represnts an abstraction of a field of the Operating Point.
	* In particular it provides an ordered view of the configuration with
	* respect to the target field.
	* It provides the following functionalities:
	*   - add Operating Points to the container
	*   - remove Operating Points to the container
	*   - retrieve a range of Operating Points according to the value
	*     of the target field
	*
	* The operation on this structure are not protected by a mutex, thus they
	* subject of data-races.
	*/
	class view_t
	{
			/**
			 * @brief typedef to the field extractor
			 */
			using extractor_t = std::function< margot_value_t (const operating_points_t::value_type& ) >;


			/**
			 * @brief the internal container type of the view
			 */
			using container_t = std::multimap< const margot_value_t, const configuration_t>;


		public:

			/**
			 * @brief typedef of a range iterator to the map
			 */
			using view_range_t = std::pair< container_t::const_iterator, container_t::const_iterator >;


			/****************************************************
			 * Constructors and destructor of the view
			 ****************************************************/

			/**
			 * @brief Default constructor
			 */
			view_t( void );


			/**
			 * @brief Named constructor that targets a parameter
			 *
			 * @param [in] parameter The reference to the parameter
			 * @param [in] points The map of configurations
			 */
			static inline view_t parameter_view( const field_name_t parameter, const configuration_map_t& points )
			{
				auto extractor = [parameter] (const operating_points_t::value_type & op)
				{
					return static_cast<margot_value_t>(op.first[parameter]);
				};
				return view_t( extractor, parameter, points, true );
			}


			/**
			 * @brief Named constructor that targets a metric
			 *
			 * @param [in] metric The reference to the metric
			 * @param [in] points The map of configurations
			 */
			static inline view_t metric_view( const field_name_t metric, const configuration_map_t& points )
			{
				auto extractor = [metric] (const operating_points_t::value_type & op)
				{
					return static_cast<margot_value_t>(op.second[metric]);
				};
				return view_t( extractor, metric, points, false );
			}

			/**
			 * @brief The copy constructor
			 *
			 * @param [in] source The source element
			 */
			view_t( const view_t& source) = default;

			/**
			 * @brief The move constructor
			 *
			 * @param [in] source The source element
			 */
			view_t(view_t&& source) = default;

			/**
			 * @brief Assign operator (Copy semantic)
			 *
			 * @param [in] source The oher element
			 */
			view_t& operator=(const view_t& source) = default;

			/**
			 * @brief Assign operator (Move semantic)
			 *
			 * @param [in] source The oher element
			 */
			view_t& operator=(view_t&& source) = default;

			/**
			 * @brief The destructor
			*/
			~view_t( void ) = default;







			/****************************************************
			 * Methods to modify the container
			 ****************************************************/


			/**
			 * @brief Adds a set of Operating Points to the view
			 *
			 * @param [in] ops The list of Operating Point to add
			 *
			 * @note
			 * It is not checked the fact that an alredy existing configuration is
			 * added again.
			 */
			void add( const operating_points_t& ops);




			/**
			 * @brief Remove a set of Operating Points from the view
			 *
			 * @param [in] ops The list of Operating Point to remove
			 *
			 * @details
			 * An attempt to remove a non-existent configuration does not throw
			 * an exception, nor it is notified anywhere else
			 */
			void remove( const operating_points_t& ops);






			/****************************************************
			 * Methods to access the container
			 ****************************************************/

			/**
			 * @brief Get the configurations between two values
			 *
			 * @param [in] a The first extreme of the range
			 * @param [out] b The second extreme of the range
			 *
			 * @return A pair of iterator to the begin/end of target sequence
			 *
			 * @details
			 * This methods return a range of configuration by means of a pait of iterator
			 * to the begin and to the end of the range of selections.
			 *
			 * The range selected by this method includes the extremes of the selection, however
			 * the "end iterator" is meant to be used in a for loop, thus it refers to the next
			 * element after the last.
			 *
			 * If the range is much lower wrt the lowest configuration, then both the iterators refer
			 * to the container.cbegin.
			 *
			 * If the range is much higher wrt the highest configuration, then both the iterators
			 * refer to the container.cend.
			 */
			inline const view_range_t range( const margot_value_t a, const margot_value_t b) const
			{
				return { view.lower_bound(std::min(a, b)), view.upper_bound(std::max(a, b)) };
			}



			/**
			 * @brief Get all the configurations
			 *
			 * @return A pair of iterator to the beginning and end of all the configurations
			 */
			inline const view_range_t range( void ) const
			{
				return { view.cbegin(), view.cend() };
			}


			/**
			 * @brief Get the interested value from an Operating Point
			 *
			 * @param [in] op The target OP
			 *
			 * @return The value of the field
			 */
			inline margot_value_t extract_op_value( const operating_point_t& op ) const
			{
				return extractor(op);
			}


			/**
			 * @brief Get the maximum known value
			 *
			 * @return The maximum value
			 */
			margot_value_t get_maximum_value( void ) const;


			/**
			 * @brief Get the minimum known value
			 *
			 * @return The minimum value
			 */
			margot_value_t get_minimum_value( void ) const;


			/**
			 * @brief Get the number of configurations known
			 *
			 * @return The size of the container
			 */
			inline std::size_t size( void ) const
			{
				return view.size();
			}

			/**
			 * @brief Retrieve the name of the target field
			 *
			 * @return The name of the field
			 *
			 * @details
			 * The name of the field is defined as follows:
			 *  - the index of the metric
			 *  - minus the index of the software knob
			 *
			 * Since the field name is an unsigned integer, for the parameters they
			 * will overflow and get a huge value, thus if the autotuner is managing
			 * really huge configuration, they might clash.
			 * However, it is really unlikely that this will happens, since they will
			 * deplept the memory first. So it is ok.
			 */
			inline field_name_t get_field_name( void ) const
			{
				return unique_name;
			}

		private:



			/**
			 * @brief The internal constructor
			 *
			 * @param [in] parameter The reference to the parameter
			 * @param [in] points The Operating Points list
			 */
			view_t( extractor_t extractor, const field_name_t field, const configuration_map_t& points, const bool is_parameter );


			/**
			 * @brief The container of the view
			 */
			container_t view;


			/**
			 * @brief The extractor of a metric
			 */
			extractor_t extractor;

			/**
			 * @brief A unique name for the field
			 */
			const field_name_t unique_name;
	};

	/**
	 * @brief Typedef for a pointer to a view_t
	 */
	using view_t_ptr = std::shared_ptr<view_t>;

}



#endif // MARGOT_ASRTM_VIEW_HEADER
