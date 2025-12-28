#pragma once

namespace Engine {
	enum Layer { 
		DEBUG = 1 << 0,
		SIMPLE = 1 << 1,
		DEFAULT = 1 << 2,
		MINIMAP = 1 << 3,
	};

	class LayerMask {
	private:
		unsigned int allowedLayers;
	public:
		LayerMask(unsigned int allowed_layers);

		// Will return true if "other" contains any of the layers in our object
		bool matchWith(const LayerMask& other);

		void enableLayer(Layer layer);
		void disableLayer(Layer layer);
	};

	class HasLayerMask {
	private:
		LayerMask layerMask;
	public:
		HasLayerMask();

		void setLayerMask(const LayerMask&& layerMask);

		LayerMask& getLayerMask();
	};
}