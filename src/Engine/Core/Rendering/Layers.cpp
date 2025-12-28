#include <Engine/Core/Rendering/Layers.hpp>

namespace Engine {
	LayerMask::LayerMask(unsigned int allowed_layers) {
		this->allowedLayers = allowed_layers;
	};

	bool LayerMask::matchWith(const LayerMask& other) {
		return (bool)(this->allowedLayers & other.allowedLayers); 
	};

	void LayerMask::enableLayer(Layer layer) {
		this->allowedLayers |= layer;
	};

	void LayerMask::disableLayer(Layer layer) {
		this->allowedLayers &= ~layer;
	};

	HasLayerMask::HasLayerMask() : layerMask(
		LayerMask(Layer::DEFAULT)
	) {};

	void HasLayerMask::setLayerMask(const LayerMask&& layerMask) {
		this->layerMask = layerMask;
	};

	LayerMask& HasLayerMask::getLayerMask() {
		return this->layerMask;
	};
}