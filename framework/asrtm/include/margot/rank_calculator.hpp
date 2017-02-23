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


#ifndef MARGOT_ASRTM_RANK_HEADER
#define MARGOT_ASRTM_RANK_HEADER

#include <cstddef>
#include <functional>
#include <cmath>

#include <margot/config.hpp>
#include <margot/operating_point.hpp>

/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	/**
	 * @brief Typedef for the value of the rank
	 */
	using rank_t = float;


	/**
	 * @brief A wrapper for a term for the rank computation on a parameter of the Operating Point
	 *
	 * @details
	 * This class is meant to represent a term to compute the rank of an Operating Point.
	 * The term refers to a parameter of the Operating Point
	 */
	class rank_parameter_t
	{
		public:
			field_name_t field;
			rank_t coefficient;
	};

	/**
	 * @brief A wrapper for a term for the rank computation on a metric of the Operating Point
	 *
	 * @details
	 * This class is meant to represent a term to compute the rank of an Operating Point.
	 * The term refers to a metric of the Operating Point
	 */
	class rank_metric_t
	{
		public:
			field_name_t field;
			rank_t coefficient;
	};


	/**
	 * @brief The direction of the optimization
	 *
	 * @details
	 * The rank is a number attached to an Operating Point that defines its goodness,
	 * however it is the user that should specify if he wants to maximize or minimize
	 * the rank.
	 */
	enum class RankObjective
	{
		Minimize,
		Maximize
	};

	/**
	 * @brief Compute the rank of an Operating Point
	 *
	 * @details
	 * This class computes the rank of an Operating Point. The definition is set
	 * from the user. This class assumes that the srtucture of the Operating Points
	 * does not change at runtime.
	 * The idea is the this class implement the operator() to compute the rank, while
	 * the user might be able to provide a custom defintion of rank thorugh the definition
	 * of a lambda function that encapsule the desired rank definition.
	 */
	class rank_calculator_t
	{
		public:

			using rank_compute_t = std::function<rank_t(const operating_point_t&)>;

			/****************************************************
			 * Constructor and Deconstructors
			 ****************************************************/

			/**
			 * @brief Default constructor
			*
			* @details
			* By default all the Operating Points have a rank equals to zero
			*/
			rank_calculator_t( void );

			/**
			 * @brief Copy constructor
			 *
			 * @param [in] source The source rank calculator
			 */
			rank_calculator_t( const rank_calculator_t& source ) = default;

			/**
			 * @brief Copy constructor
			 *
			 * @param [in] source The source rank calculator
			 */
			rank_calculator_t( rank_calculator_t&& source ) = default;

			/**
			 * @brief Assign operator (copy semantics)
			 *
			 * @param [in] source The source orank calculator
			 */
			rank_calculator_t& operator=(const rank_calculator_t& source ) = default;

			/**
			 * @brief Assign operator (move semantics)
			 *
			 * @param [in] source The source rank calculator
			 */
			rank_calculator_t& operator=( rank_calculator_t&& source ) = default;

			/**
			 * @brief Destructor
			 */
			~rank_calculator_t( void ) = default;



			/****************************************************
			 * Class public interface
			 ****************************************************/

			/**
			 * @brief Computes the rank of an Operating Point
			 *
			 * @param [in] op The target Operating Point
			 *
			 * @return its rank value
			 */
			inline rank_t operator()(const operating_point_t& op) const
			{
				return computer(op);
			}


			/**
			 * @brief Define the rank function
			 *
			 * @param [in] direction Specify if the user wants to maximize or minimize the rank value
			 * @param [in] op_fields The list of the terms that specify the rank
			 *
			 * @details
			 * The rank of an Operating Point, using the linear rank, defined as:
			 *    rank = t1.coefficient*t1.field + t2.coefficiont*t2.field + ...
			 */
			template<class ...T>
			void define_linear_rank(const RankObjective direction, const T... op_fields)
			{
				// set the sign
				const int sign = direction == RankObjective::Maximize ? -1 : 1;

				// set the lambda
				computer = [ sign, op_fields... ] (const operating_point_t& op)
				{
					return static_cast<rank_t>(sign) * compute_linear_rank(op, op_fields...);
				};
			}



			/**
			 * @brief Define the rank function
			 *
			 * @param [in] direction Specify if the user wants to maximize or minimize the rank value
			 * @param [in] op_fields The list of the terms that specify the rank
			 *
			 * @details
			 * The rank of an Operating Point, using the geometric rank, defined as:
			 *    rank = t1.field^t1.coefficient * t2.field^t2.coefficiont * ...
			 */
			template<class ...T>
			void define_geometric_rank(const RankObjective direction, const T... op_fields)
			{
				// set the sign
				const int sign = direction == RankObjective::Maximize ? -1 : 1;

				// set the lambda
				computer = [ sign, op_fields... ] (const operating_point_t& op)
				{
					return static_cast<rank_t>(sign) * compute_geometric_rank(op, op_fields...);
				};
			}




		private:

			/****************************************************
			 * The linear rank functions
			 ****************************************************/

			/**
			 * @brief Compile-recursive function to parse a metric contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] metric_term The metric term that contributes to the rank computation
			 * @param [in] remainder The remainder of the terms that compose a rank
			 *
			 * @return The partial value of the rank
			 */
			template<class ...T>
			static inline rank_t compute_linear_rank( const operating_point_t& op, const rank_metric_t& metric_term, const T... remainder )
			{
				return static_cast<rank_t>(op.second[metric_term.field]) * metric_term.coefficient + rank_calculator_t::compute_linear_rank(op, remainder... );
			}


			/**
			 * @brief Compile-recursive function to parse a parameter contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] parameter_term The parameter term that contributes to the rank computation
			 * @param [in] remainder The remainder of the terms that compose a rank
			 *
			 * @return The partial value of the rank
			 */
			template<class ...T>
			static inline rank_t compute_linear_rank( const operating_point_t& op, const rank_parameter_t& parameter_term, const T... remainder )
			{
				return static_cast<rank_t>(op.first[parameter_term.field]) * parameter_term.coefficient + rank_calculator_t::compute_linear_rank( op, remainder... );
			}


			/**
			 * @brief Compile-recursive function to parse the last metric contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] metric_term The metric term that contributes to the rank computation
			 *
			 * @return The last value of the rank
			 */
			static inline rank_t compute_linear_rank( const operating_point_t& op, const rank_metric_t& metric_term )
			{
				return static_cast<rank_t>(op.second[metric_term.field]) * metric_term.coefficient;
			}


			/**
			 * @brief Compile-recursive function to parse the last parameter contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] param_term The parameter term that contributes to the rank computation
			 *
			 * @return The last value of the rank
			 */
			static inline rank_t compute_linear_rank( const operating_point_t& op, const rank_parameter_t& parameter_term )
			{
				return static_cast<rank_t>(op.first[parameter_term.field]) * parameter_term.coefficient;
			}




			/****************************************************
			 * The geometric rank functions
			 ****************************************************/

			/**
			* @brief Compile-recursive function to parse a metric contribution
			*
			* @param [in] op The target Operating Point
			* @param [in] metric_term The metric term that contributes to the rank computation
			* @param [in] remainder The remainder of the terms that compose a rank
			*
			* @return The partial value of the rank
			*/
			template<class ...T>
			static inline rank_t compute_geometric_rank( const operating_point_t& op, const rank_metric_t& metric_term, const T... remainder )
			{
				return std::pow(static_cast<rank_t>(op.second[metric_term.field]), metric_term.coefficient) * rank_calculator_t::compute_geometric_rank( op, remainder... );
			}


			/**
			 * @brief Compile-recursive function to parse a parameter contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] parameter_term The parameter term that contributes to the rank computation
			 * @param [in] remainder The remainder of the terms that compose a rank
			 *
			 * @return The partial value of the rank
			 */
			template<class ...T>
			static inline rank_t compute_geometric_rank( const operating_point_t& op, const rank_parameter_t& parameter_term, const T... remainder )
			{
				return std::pow(static_cast<rank_t>(op.first[parameter_term.field]), parameter_term.coefficient) * rank_calculator_t::compute_geometric_rank( op, remainder... );
			}


			/**
			 * @brief Compile-recursive function to parse the last parameter contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] parameter_term The parameter term that contributes to the rank computation
			 *
			 * @return The last value of the rank
			 */
			static inline rank_t compute_geometric_rank( const operating_point_t& op, const rank_parameter_t& parameter_term )
			{
				return std::pow(static_cast<rank_t>(op.first[parameter_term.field]), parameter_term.coefficient);
			}


			/**
			 * @brief Compile-recursive function to parse the last metric contribution
			 *
			 * @param [in] op The target Operating Point
			 * @param [in] metric_term The metric term that contributes to the rank computation
			 *
			 * @return The last value of the rank
			 */
			static inline rank_t compute_geometric_rank( const operating_point_t& op, const rank_metric_t& metric_term )
			{
				return std::pow(static_cast<rank_t>(op.second[metric_term.field]), metric_term.coefficient);
			}



			/**
			 * @brief a pointer to the function that uses the lambda
			 */
			rank_compute_t computer;
	};


}


#endif // MARGOT_ASRTM_RANK_HEADER
