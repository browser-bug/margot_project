/* beholder/ici_cdt.hpp
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


#ifndef MARGOT_BEHOLDER_ICI_CDT
#define MARGOT_BEHOLDER_ICI_CDT

#include <vector>
#include <string>

#include "beholder/ici_test_data.hpp"


namespace beholder
{
  class IciCdt
  {


    private:

      static void compute_timestamps(Data_ici_test& data_test, const std::vector<std::pair <float, std::string>>& window_pair);


    public:

      static bool perform_ici_cdt(Data_ici_test& data_test, const std::vector<std::pair <float, std::string>>& window_pair);

  };


}

#endif // MARGOT_BEHOLDER_ICI_CDT