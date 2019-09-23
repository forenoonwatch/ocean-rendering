#pragma once

#include <engine/core/common.hpp>

class RenderContext;
class VertexArray;

class Ocean {
	public:
		Ocean(RenderContext& context, float oceanHeight,
				float maxAmplitude, uint32 gridLength);

		inline VertexArray& getGridArray() { return *gridArray; }

		inline float getOceanHeight() const { return oceanHeight; }
		inline float getMaxAmplitude() const { return maxAmplitude; }

		~Ocean();
	private:
		NULL_COPY_AND_ASSIGN(Ocean);

		RenderContext* context;

		VertexArray* gridArray;
		
		float oceanHeight;
		float maxAmplitude;
		uint32 gridLength;

};

