#pragma once

namespace fingera {

template<class to_initialize>
class static_initializer {
public:
	static_initializer() {
		to_initialize::init();
	}
	~static_initializer() {
		to_initialize::un_init(); 
	}
};

} // namespace fingera