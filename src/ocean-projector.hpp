#pragma once

#include "camera.hpp"

class Ocean;

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
