#pragma once
#include <string>
#include <unordered_map>

namespace Engine {
	const std::string TRANSFORM_UPDATE_EVENT = "TRANSFORM_UPDATE_EVENT";

	union EventDataPackageValue {
		float f_value;
		int i_value;
		void* ptr_value;
	};

	typedef std::unordered_map<std::string, EventDataPackageValue> EventDataPacket;
	
	class Event {
	private:
		const std::string name;
		EventDataPacket data;

	public:
		Event(const std::string& name);
		Event(const std::string& name, EventDataPacket data);

		const EventDataPacket& getData();
		const std::string& getName();
	};
};