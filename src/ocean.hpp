#pragma once

#include "indexed-model.hpp"
#include "camera.hpp"

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

		void addVertex(float, float);
		void addIndices(uint32, uint32, uint32);
		void setAdjacent(uint32, uint32, uint32);
};

class OceanProjector {
	public:
		OceanProjector(Ocean& ocean, Camera& viewCamera);

		void update();

		inline const glm::mat4& getProjectorMatrix() const { return mProjector; }
		inline const glm::vec4* getCorners() const { return corners; }
	private:
		Ocean& ocean;
		
		Camera& viewCamera;
		Camera projectorCamera;

		glm::mat4 mProjector;
		glm::vec4 corners[4];

		static const int cameraEdges[24];
};
