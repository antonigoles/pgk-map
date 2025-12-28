#include <Engine/Core/Math/MinMaxQueue.hpp>

namespace Engine {
	template <typename T>
	MinMaxQueue<T>::MinMaxQueue() {
		this->q = {};
	};
	
	template <typename T>
	void MinMaxQueue<T>::insert(T element) {
		this->q.insert(element);
	};

	template <typename T>
	void MinMaxQueue<T>::remove(T element) {
		if (!this->q.contains(element)) return;
		this->q.erase(this->q.find(element));
	};

	template <typename T>
	T MinMaxQueue<T>::getMax() {
		return *std::prev(this->q.end());
	};

	template <typename T>
	T MinMaxQueue<T>::getMin() {
		return *this->q.begin();
	};

	template <typename T>
	T MinMaxQueue<T>::getMid() {
		auto begin = this->q.begin();
		std::advance(begin, this->q.size() / 2 );
		return *(begin);
	}

	template <typename T>
	void MinMaxQueue<T>::clear() {
		this->q.clear();
	}


	template class MinMaxQueue<float>;
}