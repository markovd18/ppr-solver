#include <chrono>
#include <thread>

#include "./include/watchdog.h"

Watchdog::Watchdog(std::chrono::milliseconds timeout) : m_timeout(timeout) {}

void Watchdog::Start() {
	m_guarding = true;
	m_watchdog_thread = std::thread(&Watchdog::Run, this);
}

void Watchdog::Stop() {
	m_guarding = false;
	m_watchdog_thread.join();
}

void Watchdog::Kick(std::size_t processsed_items) {
	m_total_processed_items += processsed_items;
}

void Watchdog::Run() {
	static const std::size_t termination_threshold = 5;
	std::size_t timeout_count = 0;

	while (m_guarding) {
		std::size_t processed_old = m_total_processed_items;
		std::this_thread::sleep_for(m_timeout);
		std::size_t processed_now = m_total_processed_items;

		// if processed count increased, computation works
		if (processed_now > processed_old) {
			timeout_count = 0;
			continue;
		}

		timeout_count++;
		//std::cout << "Program inactive for " << (timeout_count * timeout.count() / 1000) << " seconds" << std::endl;
		//Terminate whole program after timeout*6 seconds
		if (timeout_count > termination_threshold) {
			/*std::cout << "Terminating, current results are: " << std::endl
				<< "Processed doubles: " << processed_items;*/
			/*final_distr.make_distribution_decision();
			final_distr.print_distribution_decision();*/
			std::terminate();
		}
	}
}