#include "ocean.hpp"

#include <engine/rendering/vertex-array.hpp>

inline static void initGrid(IndexedModel&, uint32);

Ocean::Ocean(RenderContext& context, float oceanHeight,
			float maxAmplitude, uint32 gridLength)
		: context(&context)
		, oceanHeight(oceanHeight)
		, maxAmplitude(maxAmplitude)
		, gridLength(gridLength) {
	IndexedModel grid;
	initGrid(grid, gridLength);

	gridArray = new VertexArray(context, grid, GL_STATIC_DRAW);
}

Ocean::~Ocean() {
	delete gridArray;
}

inline static void initGrid(IndexedModel& grid, uint32 gridLength) {
	const float fGridLength = (float)gridLength;
	const float ifGridLength = 1.f / (fGridLength - 1.f);

	grid.allocateElement(2); // vec2 position
	grid.allocateElement(16); // mat4 transform

	grid.setInstancedElementStartIndex(1);

	for (float y = 0; y < fGridLength; ++y) {
		for (float x = 0; x < fGridLength; ++x) {
			grid.addElement2f(0, x / (fGridLength - 1.f), y / (fGridLength - 1.f));
		}
	}

	for (int32 y = 1; y < gridLength; ++y) {
		for (int32 x = 1; x < gridLength; ++x) {
			int32 i0 = y * gridLength + x;
			int32 i1 = (y - 1) * gridLength + x;
			int32 i2 = y * gridLength + x - 1;
			int32 i3 = (y - 1) * gridLength + x - 1;

			grid.addIndices3i(i0, i1, i2);
			grid.addIndices3i(i1, i3, i2);
		}
	}
}

