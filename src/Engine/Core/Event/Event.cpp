#include <Engine/Core/Event/Event.hpp>

namespace Engine {
	Event::Event(const std::string& name) : name(name), data({}) {};

	Event::Event(const std::string& name, EventDataPacket data) : name(name), data(data) {};

	const EventDataPacket& Event::getData() {
		return this->data;
	};

	const std::string& Event::getName() {
		return this->name;
	};
};