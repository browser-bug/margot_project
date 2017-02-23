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



#ifndef MARGOT_ASRTM_KNOWLEDGE_BASE_HEADER
#define MARGOT_ASRTM_KNOWLEDGE_BASE_HEADER


#include <map>
#include <vector>
#include <string>
#include <memory>
#include <chrono>




#include "margot/operating_point.hpp"
#include "margot/view.hpp"
#include "margot/config.hpp"


/**
  * @brief The namespace for the mARGOt framework
  */
namespace margot
{




	/**
	 * @brief A class container for the knowledge of the application
	 *
	 * @details
	 * THis class is meant as a wrapper to encapsulte the class knowledge_t that
	 * stores all the information regarding the target application
	 */
	class knowledge_base_t
	{
		public:

			/**
			 * @brief A reference to the views
			 */
			using view_t_ptr = std::shared_ptr< view_t >;


			/**
			 * @brief Typedef of a vector of views
			 */
			using views_t = std::vector< view_t_ptr >;


			/**
			 * @brief Typedef for the range of the knowledge
			 */
			using knowledge_range_t = std::pair<configuration_map_t::const_iterator, configuration_map_t::const_iterator>;


			/**
			 * @brief Typedef of the version number of the knowledge base
			 *
			 * @details
			 * It is used to check if a synch between the application base and a state
			 * is required.
			 */
			using version_t = std::chrono::steady_clock::time_point;



			/**
			 * @brief Typedef for the dictionary of the fields of the Operating Points
			 */
			using translator_t = std::map< field_name_t, std::string >;


			/****************************************************
			 * Constructors and Deconstructors
			 ****************************************************/

			/**
			 * @brief Default constructor
			 */
			knowledge_base_t( void );

			/**
			 * @brief Copy constructor
			 *
			 * @param [in] source The source of the knowledge
			 */
			knowledge_base_t( const knowledge_base_t& source ) = default;

			/**
			 * @brief Move constructor
			 *
			 * @param [in] source The source of the knowledge
			 */
			knowledge_base_t( knowledge_base_t&& source ) = default;

			/**
			 * @brief Assign operator (copy semantics)
			 *
			 * @param [in] source The source of the knowledge
			 */
			knowledge_base_t& operator=(const knowledge_base_t& source ) = default;

			/**
			 * @brief Assign operator (move semantics)
			 *
			 * @param [in] source The source of the knowledge
			 */
			knowledge_base_t& operator=( knowledge_base_t&& source ) = default;

			/**
			 * @brief Destructor
			 */
			~knowledge_base_t( void ) = default;



			/****************************************************
			 * Methods to modify the knowledge
			 ****************************************************/

			/**
			 * @brief Attach a description of the Operating Point
			 *
			 * @param [in] parameters The map that names each parameter
			 * @param [in] metrics The map that names each metric
			 *
			 * @details
			 * Adding a description automatically overwrite the previous one.
			 * The name of the fields of the Operating Point are used as a
			 */
			inline void add_description( translator_t parameters, translator_t metrics )
			{
				knowledge->add_description(std::move(parameters), std::move(metrics));
			}


			/**
			 * @brief Add a list of Operating Points to knowledge base
			 *
			 * @param [in] ops The Operating Points to add
			 *
			 * @details
			 * The containers of the metric and parameter views is created the first
			 * time that the Operating Points are created
			 */
			inline void add_operating_points( const operating_points_t& ops )
			{
				knowledge->add_operating_points(ops);
			}


			/**
			 * @brief Remove Operating Points from the knowledge base
			 *
			 * @param  [in] ops The Operating Points to remove
			 *
			 * @details
			 * The containers of the metric and parameter views is cleared if all the
			 * Operating Points are removed.
			 */
			inline void remove_operating_points( const operating_points_t& ops)
			{
				knowledge->remove_operating_points(ops);
			}



			/****************************************************
			 * Methods to retrieve the knowledge
			 ****************************************************/

			/**
			 * @brief Return a reference to a parameter view
			 *
			 * @param [in] parameter_name The name of the target parameter
			 *
			 * @return A reference to the view that targets the parameter of interest
			 *
			 * @note
			 * The knowledge base creates and populate the view the first time that it
			 * is requested.
			 */
			inline view_t_ptr get_parameter_view( const field_name_t param_name )
			{
				return knowledge->get_parameter_view(param_name);
			}


			/**
			 * @brief Return a reference to a metric view
			 *
			 * @param [in] metric_name The name of the target metric
			 *
			 * @return A reference to the view that targets the metric of interest
			 *
			 * @note
			 * The knowledge base creates and populate the view the first time that it
			 * is requested.
			 */
			inline view_t_ptr get_metric_view( const field_name_t metric_name )
			{
				return knowledge->get_metric_view(metric_name);
			}


			/**
			 * @brief Return the version of the knowledge
			 *
			 * @return The version number
			 */
			inline version_t get_version( void ) const
			{
				return knowledge->get_version();
			}


			/**
			 * @brief Translate a parameter name from a numeric value to a string
			 *
			 * @return A string that represents the name of the parameters
			 */
			inline std::string get_parameter_name( const field_name_t param_name ) const
			{
				return knowledge->get_parameter_name(param_name);
			}


			/**
			 * @brief Translate a metric name from a numeric value to a string
			 *
			 * @return A string that represents the name of the metric
			 */
			inline std::string get_metric_name( const field_name_t metric_name ) const
			{
				return knowledge->get_metric_name(metric_name);
			}


			/**
			 * @brief Retrive a pair of iterator to the configuration map
			 *
			 * @return A pair of iterator, where the first is the begin, the second the end
			 */
			inline const knowledge_range_t get_knowlege_range( void ) const
			{
				return knowledge->get_knowlege_range();
			}

			/**
			 * @brief Retrieve a reference to an Operating Point
			 *
			 * @param [in] configuration The target configuration
			 *
			 * @return A reference to the whole Operating Point
			 */
			inline const operating_point_t get_operating_point( const configuration_t& configuration ) const
			{
				return knowledge->get_operating_point(configuration);
			}


			/**
			 * @brief Get the number of Operating Points
			 *
			 * @return The number of OPs
			 */
			inline std::size_t size( void ) const
			{
				return knowledge->size();
			}

			/**
			 * @brief Test whether the Knowledge is empty or not
			 *
			 * @return True, if there are no Operating Point in the knowledge
			 */
			inline bool empty( void ) const
			{
				return knowledge->empty();
			}


			/**
			 * @brief The description of the application behaviour
			 *
			 * @details
			 * This class represents the knowledge of the application behavior, defined
			 * as the performance reached by the block of code under investigation.
			 * This class contains the following information:
			 *   - the list of Operating Points
			 *   - the list of views to focus on a single field of the Operating Point
			 *   - the structure of an Operating Points
			 */
			class knowledge_t
			{

				public:

					/****************************************************
					 * Constructors and Deconstructors
					 ****************************************************/

					/**
					 * @brief Default constructor
					 */
					knowledge_t( void ): version(std::chrono::steady_clock::now()) {}

					/**
					 * @brief Copy constructor
					 *
					 * @param [in] source The source of the knowledge
					 */
					knowledge_t( const knowledge_t& source ) = default;

					/**
					 * @brief Copy constructor
					 *
					 * @param [in] source The source of the knowledge
					 */
					knowledge_t( knowledge_t&& source ) = default;

					/**
					 * @brief Assign operator (copy semantics)
					 *
					 * @param [in] source The source of the knowledge
					 */
					knowledge_t& operator=(const knowledge_t& source ) = default;

					/**
					 * @brief Assign operator (move semantics)
					 *
					 * @param [in] source The source of the knowledge
					 */
					knowledge_t& operator=( knowledge_t&& source ) = default;




					/****************************************************
					 * Methods to modify the knowledge
					 ****************************************************/

					/**
					 * @brief Attach a description of the Operating Point
					 *
					 * @param [in] parameters The map that names each parameter
					 * @param [in] metrics The map that names each metric
					 *
					 * @details
					 * Adding a description automatically overwrite the previous one.
					 * The name of the fields of the Operating Point are used as a
					 */
					void add_description( translator_t parameters, translator_t metrics );


					/**
					 * @brief Add a list of Operating Points to knowledge base
					 *
					 * @param [in] ops The Operating Points to add
					 *
					 * @details
					 * The containers of the metric and parameter views is created the first
					 * time that the Operating Points are created
					 */
					void add_operating_points( const operating_points_t& ops );


					/**
					 * @brief Remove Operating Points from the knowledge base
					 *
					 * @param  [in] ops The Operating Points to remove
					 *
					 * @details
					 * The containers of the metric and parameter views is cleared if all the
					 * Operating Points are removed.
					 */
					void remove_operating_points( const operating_points_t& ops);



					/****************************************************
					 * Methods to retrieve the knowledge
					 ****************************************************/

					/**
					 * @brief Return a reference to a parameter view
					 *
					 * @param [in] parameter_name The name of the target parameter
					 *
					 * @return A reference to the view that targets the parameter of interest
					 *
					 * @note
					 * The knowledge base creates and populate the view the first time that it
					 * is requested.
					 */
					view_t_ptr get_parameter_view( const field_name_t param_name );


					/**
					 * @brief Return a reference to a metric view
					 *
					 * @param [in] metric_name The name of the target metric
					 *
					 * @return A reference to the view that targets the metric of interest
					 *
					 * @note
					 * The knowledge base creates and populate the view the first time that it
					 * is requested.
					 */
					view_t_ptr get_metric_view( const field_name_t metric_name );


					/**
					 * @brief Return the version of the knowledge
					 *
					 * @return The version number
					 */
					inline version_t get_version( void ) const
					{
						return version;
					}

					/**
					 * @brief Translate a parameter name from a numeric value to a string
					 *
					 * @return A string that represents the name of the parameters
					 */
					inline std::string get_parameter_name( const field_name_t param_name ) const
					{
						return parameter_translator.at(param_name);
					}

					/**
					 * @brief Translate a metric name from a numeric value to a string
					 *
					 * @return A string that represents the name of the metric
					 */
					inline std::string get_metric_name( const field_name_t metric_name ) const
					{
						return metric_translator.at(metric_name);
					}


					/**
					 * @brief Retrive a pair of iterator to the configuration map
					 *
					 * @return A pair of iterator, where the first is the begin, the second the end
					 */
					inline const knowledge_range_t get_knowlege_range( void ) const
					{
						return knowledge_range_t(knowledge.cbegin(), knowledge.cend());
					}


					/**
					 * @brief Retrieve a reference to an Operating Point
					 *
					 * @param [in] configuration The target configuration
					 *
					 * @return A reference to the whole Operating Point
					 */
					inline const operating_point_t get_operating_point( const configuration_t& configuration ) const
					{
						return operating_point_t{configuration, knowledge.at(configuration)};
					}


					/**
					 * @brief Get the number of Operating Points
					 *
					 * @return The number of OPs
					 */
					inline std::size_t size( void ) const
					{
						return knowledge.size();
					}

					/**
					 * @brief Test whether the Knowledge is empty or not
					 *
					 * @return True, if there are no Operating Point in the knowledge
					 */
					inline bool empty( void ) const
					{
						return knowledge.empty();
					}

				private:

					/**
					 * @brief The list of all the known configurations
					 */
					configuration_map_t knowledge;

					/**
					 * @brief The list of all the views on the parameters
					 */
					views_t parameter_views;

					/**
					 * @brief The list of all the views on the metrics
					 */
					views_t metric_views;

					/**
					 * @biref The version of the Operating Points
					 *
					 * @details
					 * It is used to check if a synch between the application base and a state
					 * is required.
					 */
					version_t version;

					/**
					 * @brief The parameter translator
					 */
					translator_t parameter_translator;

					/**
					 * @brief The metric translator
					 */
					translator_t metric_translator;

			};

			/**
			 * @brief Typedef to a reference of the knowledge
			 */
			using knowledge_t_ptr = std::shared_ptr<knowledge_t>;


			/**
			 * @brief Return a reference to the internal knowledge
			 */
			inline knowledge_t_ptr get_knowledge( void ) const
			{
				return knowledge;
			}


		private:

			/**
			 * @brief A reference to the knowledge
			 */
			knowledge_t_ptr knowledge;
	};



}


#endif // MARGOT_ASRTM_KNOWLEDGE_BASE_HEADER
