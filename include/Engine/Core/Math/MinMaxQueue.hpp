#pragma once
#include <set>

namespace Engine {
	template <typename T>
	class MinMaxQueue {
	public:
		MinMaxQueue();
		
		std::multiset<T> q;

		void insert(T element);
		void remove(T element);
		void clear();

		T getMax();
		T getMin();
		T getMid();
	};
}