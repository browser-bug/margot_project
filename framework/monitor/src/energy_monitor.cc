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

#include <margot/energy_monitor.hpp>


#include <functional>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>

/****************************************************
 * Helper functions
 ****************************************************/


std::size_t get_number_packages( void )
{
	// get the maximum number of packages
	std::size_t max_number_packages = 0;
	DIR* dp;


	// guess the right folder
	bool folder_exists = true;

	while (folder_exists)
	{
		// guess the folder name
		const std::string folder_name("/sys/class/powercap/intel-rapl/intel-rapl:" +
		                              std::to_string(max_number_packages));

		// try to open the directory
		dp = opendir(folder_name.c_str());

		// check if it does not exists
		if (dp == NULL)
		{
			folder_exists = false;
		}
		else
		{
			// store the information
			++max_number_packages;

			// close the directory
			closedir(dp);
		}
	}

	return max_number_packages;
}


std::vector<std::pair<std::string, unsigned long long int>> get_file_from_package(const std::string domain, const std::size_t package)
{
	// declare the result
	std::vector<std::pair<std::string, unsigned long long int>> target_file_list;

	// otherwise check which files are in the target domain, not needed in the "package" domain
	std::size_t domain_index = 0;

	// find all the target files
	bool found_file = true;

	while (found_file)
	{
		// open the file
		const std::string file_name("/sys/class/powercap/intel-rapl/intel-rapl:" +
		                            std::to_string(package) +
		                            (domain != "package" ? "/intel-rapl:" + std::to_string(package) + ":" + std::to_string(domain_index) : "") +
		                            "/name");
		std::ifstream name_stream(file_name);

		// check if it esists
		if (name_stream.good())
		{

			// try to retrieve the content
			const std::string domain_name( (std::istreambuf_iterator<char>(name_stream)),
			                               (std::istreambuf_iterator<char>()           ));

			// check if the domain is correct
			if (domain_name.compare(0, domain.size(), domain) == 0)
			{
				const std::string file_name_to_be_read("/sys/class/powercap/intel-rapl/intel-rapl:" +
				                                       std::to_string(package) +
				                                       (domain != "package" ? "/intel-rapl:" + std::to_string(package) + ":" + std::to_string(domain_index) : "") +
				                                       "/energy_uj");
				std::ifstream ifs("/sys/class/powercap/intel-rapl/intel-rapl:" +
				                  std::to_string(package) +
				                  (domain != "package" ? "/intel-rapl:" + std::to_string(package) + ":" + std::to_string(domain_index) : "") +
				                  "/max_energy_range_uj");
				const std::string content( (std::istreambuf_iterator<char>(ifs)),
				                           (std::istreambuf_iterator<char>()  ));
				const unsigned long long int max = std::stoull(content);
				target_file_list.emplace_back(std::make_pair(file_name_to_be_read, max));
			}

			// eventually increment the counter
			if (domain.compare("package") != 0)
			{
				++domain_index;
			}
			else
			{
				break;
			}
		}

		else
		{
			found_file = false;
		}
	}

	return std::move(target_file_list);
}




std::vector<std::pair<std::string, unsigned long long int>> get_interested_file_list(const std::size_t max_number_packages,
        const std::string domain,
        const std::vector<int> target_packages = {} )
{
	// initialize the list of files
	std::vector<std::pair<std::string, unsigned long long int>> target_file_list;

	// get all the files for the target domains
	for ( std::size_t package_index = 0; package_index < max_number_packages; ++package_index )
	{
		// check if we need to skip some package
		if (!target_packages.empty())
		{
			for ( const auto& element : target_packages)
			{
				if (package_index == element)
				{
					const auto target_files = get_file_from_package(domain, package_index);
					target_file_list.insert(target_file_list.begin(), target_files.cbegin(), target_files.cend());
					break;
				}
			}
		}
		else
		{
			// append the new files
			const auto target_files = get_file_from_package(domain, package_index);
			target_file_list.insert(target_file_list.begin(), target_files.cbegin(), target_files.cend());
		}
	}

	return target_file_list;
}


std::vector <std::pair<unsigned long long int, unsigned long long int>> get_measure(const std::vector<std::pair<std::string, unsigned long long int>> files )
{
	std::vector <std::pair<unsigned long long int, unsigned long long int>> measure;

	for ( const auto& read_file : files )
	{
		std::ifstream ifs(read_file.first);
		const std::string content( (std::istreambuf_iterator<char>(ifs)),
		                           (std::istreambuf_iterator<char>()  ));
		measure.push_back(std::make_pair(std::stoull(content), read_file.second));
	}

	return measure;
}




/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	// Default constructor
	energy_monitor_t::energy_monitor_t(const std::size_t window_size, const std::size_t min_size):
		energy_monitor_t(energy_monitor_t::Domain::Cores, window_size, min_size) {}


	// Domain specialized constructor
	energy_monitor_t::energy_monitor_t(const Domain interested_domain, const std::size_t window_size, const std::size_t min_size, const std::vector<int> target_packages):
		monitor_t<value_type>(window_size, min_size), previous_measure(0), started(false)
	{
		// fetch the name of the target metric
		std::string target_name;

		switch (interested_domain)
		{
			case Domain::Cores:
				target_name = "core";
				break;

			case Domain::Uncores:
				target_name = "uncore";
				break;

			case Domain::Ram:
				target_name = "dram";
				break;

			case Domain::Package:
				target_name = "package";
				break;

			default:
				throw std::runtime_error("Error: unknown RAPL domain");
		}

		// get the number of packages
		const auto maximum_number_packages = get_number_packages();

		if (maximum_number_packages == 0)
		{
			extractor = [] (void) -> std::vector <std::pair<unsigned long long int, unsigned long long int>>
			{
				throw std::runtime_error("Error: unable to find the RAPL file hierarchy");
			};
			return;
		}


		// get the target files that should be parsed to retrieve the informations
		const auto target_files = get_interested_file_list(maximum_number_packages, target_name, target_packages);

		if ( target_files.empty() )
		{
			extractor = [] (void) -> std::vector <std::pair<unsigned long long int, unsigned long long int>>
			{
				throw std::runtime_error("Error: no information available for the target monitor");
			};
			return;
		}

		// create the extractor
		extractor = [target_files] (void) -> std::vector <std::pair<unsigned long long int, unsigned long long int>>
		{
			return get_measure(target_files);
		};
	}



	void energy_monitor_t::start( void )
	{
		if (!started)
		{
			previous_measure = extractor();
			started = true;
		}
	}


	void energy_monitor_t::stop( void )
	{
		if (started)
		{
			std::vector <std::pair<unsigned long long int, unsigned long long int>> actual_values = extractor();
			value_type accumulator = 0;

			for (int i = 0; i < previous_measure.size(); i++)
			{
				if (actual_values.at(i).first < previous_measure.at(i).first)
				{
					accumulator += static_cast<long double>(actual_values.at(i).second - previous_measure.at(i).first + actual_values.at(i).first ) / 1000000;
				}
				else
				{
					accumulator += static_cast<long double>(actual_values.at(i).first - previous_measure.at(i).first ) / 1000000;
				}
			}

			push(accumulator);
			started = false;
		}
	}

}
