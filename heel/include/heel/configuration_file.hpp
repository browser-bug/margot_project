#ifndef HEEL_CONFIGURATION_FILE_HDR
#define HEEL_CONFIGURATION_FILE_HDR

#include <filesystem>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace margot {
namespace heel {

	class configuration_file {
		boost::property_tree::ptree content;

	public:

		// I/O functions from file
		void load( const std::filesystem::path& file_path );
		void store( const std::filesystem::path& where ) const;

		// I/O functions from std::string
		void load( const std::string& description );
		std::string to_string( void ) const;
	};

} // namespace heel
} // namespace margot

#endif // HEEL_CONFIGURATION_FILE_HDR