#include "config.hpp"

namespace Hydra {

class Hydra {

	friend Hydra::Config;

public:

	Hydra();
	virtual ~Hydra();

	void config(std::string file);
	void daemon();

private:

	// Global configuration

	Hydra::Config m_config;

	// Client connection ports (e.g. SSL and none SSL)

	std::list<Hydra::Client> m_clients;

	// Server connections (subdomains)

	std::list<Hydra::Server> m_servers;

};

}

