#pragma once

#include "indexed-model.hpp"

class Ocean : public IndexedModel {
	public:
		Ocean(float oceanHeight, float maxAmplitude, uint32 gridLength);

		inline float getOceanHeight() const { return oceanHeight; }
		inline float getMaxAmplitude() const { return maxAmplitude; }
	private:
		NULL_COPY_AND_ASSIGN(Ocean);

		float oceanHeight;
		float maxAmplitude;
		uint32 gridLength;

		void initGrid();
};

