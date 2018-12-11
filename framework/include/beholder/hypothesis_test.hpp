/* beholder/hypothesis_test.hpp
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


#ifndef MARGOT_BEHOLDER_HYP_TEST
#define MARGOT_BEHOLDER_HYP_TEST

#include <vector>
#include <string>
#include <unordered_map>




namespace beholder
{
  class HypTest
  {


    private:

    public:

      static bool perform_hypothesis_test(const std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map);

  };


}

#endif // MARGOT_BEHOLDER_HYP_TEST
