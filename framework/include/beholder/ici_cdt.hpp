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
#include <sys/stat.h> // to create directories, only for linux systems


#include "beholder/ici_test_data.hpp"
//#include "beholder/common_objects_beholder.hpp"


namespace beholder
{
  class IciCdt
  {


    private:

      inline bool create_folder( const std::string& path )
      {
        int rc = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        return rc == 0 || errno == EEXIST;
      }


    public:

      static bool perform_ici_cdt(Data_ici_test& data_test, const std::vector<residual_struct>& window_pair,
                                  std::unordered_map<std::string, output_files>& output_files_map);

  };


}

#endif // MARGOT_BEHOLDER_ICI_CDT
