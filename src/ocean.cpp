#include "ocean.hpp"

Ocean::Ocean(float oceanHeight, float maxAmplitude, uint32 gridLength)
		: oceanHeight(oceanHeight)
		, maxAmplitude(maxAmplitude)
		, gridLength(gridLength) {
	allocateElement(2); // vec2 position

	setInstancedElementStartIndex(1);
	allocateElement(16); // mat4 transform;

	initGrid();
}

inline void Ocean::initGrid() {
	const float fGridLength = (float)gridLength;
	const float ifGridLength = 1.f / (fGridLength - 1.f);

	for (float y = 0; y < fGridLength; ++y) {
		for (float x = 0; x < fGridLength; ++x) {
			addElement2f(0, x / (fGridLength - 1.f), y / (fGridLength - 1.f));
		}
	}

	for (int32 y = 1; y < gridLength; ++y) {
		for (int32 x = 1; x < gridLength; ++x) {
			int32 i0 = y * gridLength + x;
			int32 i1 = (y - 1) * gridLength + x;
			int32 i2 = y * gridLength + x - 1;
			int32 i3 = (y - 1) * gridLength + x - 1;

			addIndices3i(i0, i1, i2);
			addIndices3i(i1, i3, i2);
		}
	}
}

