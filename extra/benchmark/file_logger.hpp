#ifndef ARGO_BENCHMARK_LOGGER_HEADER
#define ARGO_BENCHMARK_LOGGER_HEADER

#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <mutex>

#define MAXIMUM_FIELD_SIZE 20


namespace margot
{



	enum class Format
	{
		CSV,
		PLAIN,
	};


	class logger_t
	{
		public:
			~logger_t(void)
			{
				if (out.is_open())
				{
					out.close();
				}

				if (out_readable.is_open())
				{
					out_readable.close();
				}
			}

			template<class ...T> void open(const std::string& file_name,
			                               const Format& out_format, const T... header)
			{
				// check if we need to close the previous files
				if (out.is_open())
				{
					out.close();
				}

				if (out_readable.is_open())
				{
					out_readable.close();
				}

				// open the file
				out.open(file_name, std::ios::out | std::ios::trunc);
				out_readable.open(file_name + ".readable", std::ios::out | std::ios::trunc);
				// store the format
				format = out_format;
				// write the header
				log_internal("Timestamp", header...);
			}

			template<class ...T> void write(const T... args)
			{
				// lock the operation
				std::lock_guard<std::mutex> lock(mutex);
				// get the current time
				std::chrono::time_point<std::chrono::steady_clock> now =
				    std::chrono::steady_clock::now();
				// log the line
				log_internal(std::chrono::duration_cast<std::chrono::microseconds>
				             (now.time_since_epoch()).count(), args...);
			}


		private:

			template<class Y, class ...T> void log_internal(const Y& firstArgument,
			        const T... remainder)
			{
				out << firstArgument;

				if (format == Format::CSV)
				{
					out << logger_t::csv_element_separator;
				}

				out << ' ';
				std::stringstream dirty_what;
				dirty_what << firstArgument;
				std::string what = dirty_what.str();

				if (what.size() < MAXIMUM_FIELD_SIZE)
				{
					what.insert(what.end(), MAXIMUM_FIELD_SIZE - what.size(), ' ');
				}
				else if (what.size() > MAXIMUM_FIELD_SIZE)
				{
					what.erase(what.begin(), what.end() - MAXIMUM_FIELD_SIZE);
				}

				out_readable << what << ' ';
				log_internal(remainder...);
			}

			template<class T> void log_internal(const T lastArgument)
			{
				out << lastArgument;

				if (format == Format::CSV)
				{
					out << logger_t::csv_row_separator;
				}

				std::stringstream dirty_what;
				dirty_what << lastArgument;
				std::string what = dirty_what.str();

				if (what.size() < MAXIMUM_FIELD_SIZE)
				{
					what.insert(what.begin(), MAXIMUM_FIELD_SIZE - what.size(), ' ');
				}
				else if (what.size() > MAXIMUM_FIELD_SIZE)
				{
					what.erase(what.begin() + MAXIMUM_FIELD_SIZE, what.end());
				}

				out_readable << what << ' ';
				out << std::endl;
				out_readable << std::endl;
			}


			std::mutex mutex;
			std::fstream out;
			std::fstream out_readable;
			Format format;


			static const char csv_element_separator = ',';
			static const char csv_row_separator = ';';
	};




} // namespace argo


#endif // ARGO_BENCHMARK_LOGGER_HEADER
