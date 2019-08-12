#pragma once

#include "indexed-model.hpp"
#include "camera.hpp"

class Ocean : public IndexedModel {
	public:
		Ocean(float oceanHeight, uint32 gridLength);

		inline float getOceanHeight() const { return oceanHeight; }
	private:
		float oceanHeight;
		uint32 gridLength;

		void initGrid();

		void addVertex(float, float);
		void addIndices(uint32, uint32, uint32);
		void setAdjacent(uint32, uint32, uint32);
};

class OceanProjector {
	public:
		OceanProjector(Ocean& ocean, Camera& viewCamera);

		void update();

		inline const glm::mat4& getProjectorMatrix() const { return mProjector; }
	private:
		Ocean& ocean;
		
		Camera& viewCamera;
		Camera projectorCamera;

		glm::mat4 mProjector;

		static const int cameraEdges[24];
};
