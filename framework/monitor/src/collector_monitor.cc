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

#include <margot/collector_monitor.hpp>
#include <stdexcept>

extern "C" {
#include <antarex_collector.h>
}




class collector_wrapper: public margot::collector_monitor_t::collector_interface
{
	public:

		collector_wrapper(const std::string topic, const std::string address, const int port)
		{
			handler = { NULL, NULL, false, 0, 0, 0, {0}, {0} };
			topic_name_c = new char[topic.length() + 1];
			address_c = new char[address.length() + 1];
			strcpy(topic_name_c, topic.c_str());
			strcpy(address_c, address.c_str());
			handler.mqtt_topic = topic_name_c;

			if (collector_init(&handler, address_c, port))
			{
				throw std::runtime_error("error: unable to initialize the collector monitor");
			}
		}

		~collector_wrapper( void )
		{
			delete [] topic_name_c;
			delete [] address_c;
			collector_clean(&handler);
		}

		void start( void )
		{
			if (collector_start(&handler))
			{
				throw std::runtime_error("error: unable to start the collector measure");
			}
		}

		void stop( void )
		{
			if (collector_end(&handler))
			{
				throw std::runtime_error("error: unable to end the collector measure");
			}
		}

		margot::collector_monitor_t::value_type get( void )
		{
			return handler.mean_val;
		}


	private:
		struct collector_val handler;
		char* topic_name_c;
		char* address_c;
};




margot::collector_monitor_t::collector_monitor_t(const std::size_t window_size,
        const std::size_t min_size):
	margot::monitor_t<margot::collector_monitor_t::value_type>(window_size, min_size)
{
	interface = nullptr;
	started = false;
}



margot::collector_monitor_t::collector_monitor_t(const std::string topic,
        const std::string address,
        const int port,
        const std::size_t window_size,
        const std::size_t min_size):
	collector_monitor_t(window_size, min_size)
{
	interface.reset(new collector_wrapper(topic, address, port));
}




void margot::collector_monitor_t::start( void )
{
	if (!started)
	{
		interface->start();
		started = true;
	}
}


void margot::collector_monitor_t::stop( void )
{
	if (started)
	{
		interface->stop();
		started = false;
		push(interface->get());
	}
}
