#pragma once
#include <string>
#include <vector>
#include <Engine/Core/Event/Event.hpp>
#include <functional>

namespace Engine {
	typedef std::function<void(EventDataPacket)> EventHandlerFunction;

	class EventDispatcher {
	private:

		std::unordered_map<std::string, std::vector<EventHandlerFunction>> eventHandlers;
	public:
		EventDispatcher();
		
		void disptach(Event& event);

		void on(const std::string& event_name, const EventHandlerFunction& handler);
	};
};