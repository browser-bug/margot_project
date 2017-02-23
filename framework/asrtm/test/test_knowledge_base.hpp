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

#include <stdexcept>
#include <iostream>

#include <cxxtest/TestSuite.h>

#include <margot/config.hpp>
#include <margot/operating_point.hpp>
#include <margot/knowledge_base.hpp>

class KnowledgeTest : public CxxTest::TestSuite
{

		margot::operating_points_t points_five;

	public:

		void setUp( void )
		{
			// the initial list
			points_five =
			{
				{
					{1},
					{1.0f, 1.0f, 5.0f}
				},
				{
					{2},
					{1.0f, 2.0f, 4.0f}
				},
				{
					{3},
					{1.0f, 3.0f, 3.0f}
				},
				{
					{4},
					{1.0f, 4.0f, 2.0f}
				},
				{
					{5},
					{1.0f, 5.0f, 1.0f}
				},
			};
		}


		void test_kb_creation_empty( void )
		{
			// create an empty knowledge
			margot::knowledge_base_t base;
		}

		void test_kb_translation( void )
		{
			// create the translation
			margot::knowledge_base_t::translator_t param_desc =
			{
				{0, "param1"},
				{1, "param2"},
				{2, "param3"}
			};
			margot::knowledge_base_t::translator_t metric_desc =
			{
				{0, "metric1"},
				{1, "metric2"},
				{2, "metric3"}
			};

			// add it to the view
			margot::knowledge_base_t base;
			base.add_description(param_desc, metric_desc);

			// check the fileds
			TS_ASSERT(base.get_parameter_name(0).compare("param1") == 0);
			TS_ASSERT(base.get_parameter_name(1).compare("param2") == 0);
			TS_ASSERT(base.get_parameter_name(2).compare("param3") == 0);
			TS_ASSERT(base.get_metric_name(0).compare("metric1") == 0);
			TS_ASSERT(base.get_metric_name(1).compare("metric2") == 0);
			TS_ASSERT(base.get_metric_name(2).compare("metric3") == 0);
		}

		void test_kb_add_ops( void )
		{
			// define the op list
			margot::knowledge_base_t base;
			base.add_operating_points(points_five);

			// get a view on the selcted op
			auto view = base.get_metric_view(1);

			// check the op
			auto op_range = view->range();
			auto it = op_range.first;
			TS_ASSERT_EQUALS(it->second[0], 1);
			++it;
			TS_ASSERT_EQUALS(it->second[0], 2);
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);
			++it;
			TS_ASSERT_EQUALS(it->second[0], 4);
			++it;
			TS_ASSERT_EQUALS(it->second[0], 5);
			++it;
			TS_ASSERT_EQUALS(it, op_range.second);
		}

		void test_kb_remove_ops( void )
		{
			// define the kb
			margot::knowledge_base_t base;
			base.add_operating_points(points_five);

			// create a view on the second metric
			auto view1 = base.get_parameter_view(0);

			// remove the operating points
			margot::operating_points_t removed_ops =
			{
				{
					{2},
					{1.0f, 5.0f, 1.0f}
				},
				{
					{3},
					{1.0f, 5.0f, 1.0f}
				},
				{
					{4},
					{1.0f, 5.0f, 1.0f}
				}
			};

			// remove the Operating Points
			base.remove_operating_points(removed_ops);

			// add a view on the metric
			auto view2 = base.get_metric_view(0);

			// get the two ranges
			auto op_range1 = view1->range();
			auto op_range2 = view2->range();


			// perform the test
			auto it = op_range1.first;
			TS_ASSERT_EQUALS(it->second[0], 1);
			++it;
			TS_ASSERT_EQUALS(it->second[0], 5);
			++it;
			TS_ASSERT_EQUALS(it, op_range1.second);


			auto it2 = op_range2.first;
			TS_ASSERT_EQUALS(it2->second[0], 1);
			++it2;
			TS_ASSERT_EQUALS(it2->second[0], 5);
			++it2;
			TS_ASSERT_EQUALS(it2, op_range2.second);
		}


		void test_kb_version( void )
		{
			// define the op list
			margot::knowledge_base_t base;
			auto version =  base.get_version();
			TS_ASSERT_LESS_THAN(0, version.time_since_epoch().count());
			base.add_operating_points(points_five);
			TS_ASSERT_LESS_THAN(version, base.get_version());
			version = base.get_version();

			// remove the operating points
			margot::operating_points_t removed_ops =
			{
				{
					{2},
					{1.0f, 5.0f, 1.0f}
				},
				{
					{3},
					{1.0f, 5.0f, 1.0f}
				},
				{
					{4},
					{1.0f, 5.0f, 1.0f}
				}
			};
			base.remove_operating_points(removed_ops);
			TS_ASSERT_LESS_THAN(version, base.get_version());
		}


};
