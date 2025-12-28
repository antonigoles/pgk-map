#include <Engine/Core/Event/EventDispatcher.hpp>

namespace Engine {
	EventDispatcher::EventDispatcher() {};
	
	void EventDispatcher::disptach(Event& event) {
		if (!this->eventHandlers.contains(event.getName())) return;

		for (auto& handler : this->eventHandlers[event.getName()]) {
			handler(event.getData());
		}
	};

	void EventDispatcher::on(const std::string& event_name, const EventHandlerFunction& handler) {
		this->eventHandlers[event_name].push_back(handler);
	};
};