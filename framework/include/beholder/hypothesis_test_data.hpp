/* beholder/hypothesis_test_data.hpp
 * Copyright (C) 2018 Alberto Bendin
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


#ifndef MARGOT_BEHOLDER_HYPOTHESIS_TEST_DATA
#define MARGOT_BEHOLDER_HYPOTHESIS_TEST_DATA

namespace beholder
{


  // data structure for the hypothesis test related to each application-metric pair
  struct Hypothesis_test_data
  {
    std::string client_id;
    std::string timestamp;
    std::vector<std::string> metric_fields_vec;
    std::vector<float> metrics_vec;
    std::vector<float> estimates_vec;

  };

}

#endif // MARGOT_BEHOLDER_HYPOTHESIS_TEST_DATA
