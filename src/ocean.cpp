#include "ocean.hpp"

#include <cfloat>

#include <GLM/gtc/matrix_transform.hpp>

Ocean::Ocean(float oceanHeight, float maxAmplitude, uint32 gridLength)
		: oceanHeight(oceanHeight)
		, maxAmplitude(maxAmplitude)
		, gridLength(gridLength) {
	allocateElement(2); // vec2 position
	allocateElement(4); // vec4 adjacent local-space positions

	setInstancedElementStartIndex(2);
	allocateElement(2 * 16); // mat4 transforms[2];

	initGrid();
}

inline void Ocean::initGrid() {
	const float fGridLength = (float)gridLength;

	for (float y = 0; y < fGridLength; ++y) {
		for (float x = 0; x < fGridLength; ++x) {
			addVertex(x / (fGridLength - 1.f), y / (fGridLength - 1.f));
		}
	}

	for (int32 y = 1; y < gridLength; ++y) {
		for (int32 x = 1; x < gridLength; ++x) {
			int32 i0 = y * gridLength + x;
			int32 i1 = (y - 1) * gridLength + x;
			int32 i2 = y * gridLength + x - 1;
			int32 i3 = (y - 1) * gridLength + x - 1;

			addIndices(i2, i1, i0);
			addIndices(i2, i3, i1);
		}
	}
}

inline void Ocean::addVertex(float x, float y) {
	addElement2f(0, x, y);
	addElement4f(1, 0.f, 0.f, 0.f, 0.f);
}

inline void Ocean::addIndices(uint32 a, uint32 b, uint32 c) {
	addIndices3i(c, b, a);

	setAdjacent(a, b, c);
	setAdjacent(b, a, c);
	setAdjacent(c, a, b);
}

inline void Ocean::setAdjacent(uint32 a, uint32 b, uint32 c) {
	float aX = getElement(0, 2 * a), aY = getElement(0, 2 * a + 1);
	float bX = getElement(0, 2 * b), bY = getElement(0, 2 * b + 1);
	float cX = getElement(0, 2 * c), cY = getElement(0, 2 * c + 1);

	setElement4f(1, a, bX - aX, bY - aY, cX - aX, cY - aY);
}

OceanProjector::OceanProjector(Ocean& ocean, Camera& viewCamera)
		: ocean(ocean)
		, viewCamera(viewCamera)
		, projectorCamera(viewCamera.getFieldOfView(), viewCamera.getAspectRatio(),
				viewCamera.getZNear(), viewCamera.getZFar())
		, mProjector(1.f) {}

void OceanProjector::update() {
	projectorCamera.update(viewCamera);

	glm::vec3 frustum[8];

	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;
	bool lookingAtOcean = false;

	for (int32 z = -1, i = 0; z <= 1; z += 2) {
		for (int32 y = -1; y <= 1; y += 2) {
			for (int32 x = -1; x <= 1; x += 2) {
				glm::vec4 p = projectorCamera.getInverseVP() * glm::vec4(x, y, z, 1.f);
				p /= p.w;
				frustum[i++] = glm::vec3(p);

				if (p.y >= -ocean.getMaxAmplitude() && p.y <= ocean.getMaxAmplitude()) {
					if (p.x < minX) {
						minX = p.x;
					}

					if (p.x > maxX) {
						maxX = p.x;
					}

					if (p.y < minY) {
						minY = p.y;
					}

					if (p.y > maxY) {
						maxY = p.y;
					}

					lookingAtOcean = true;
				}
			}
		}
	}

	for (uint32 i = 0; i < ARRAY_SIZE_IN_ELEMENTS(cameraEdges); i += 2) {
		const glm::vec3& a = frustum[cameraEdges[i]];
		const glm::vec3& b = frustum[cameraEdges[i + 1]];
		
		glm::vec3 v = b - a;
		const float m = glm::dot(v, v);

		for (float j = -1.f; j <= 1.f; j += 2.f) {
			const float t = (ocean.getOceanHeight() + j * ocean.getMaxAmplitude() - a.y) / v.y;

			if (t >= 0.f && t <= m) {
				glm::vec4 p = projectorCamera.getViewProjection() * glm::vec4(a + v * t, 1.f);

				if (p.w <= 0.0001f) {
					continue;
				}

				p /= p.w;

				if (p.x < minX) {
					minX = p.x;
				}

				if (p.x > maxX) {
					maxX = p.x;
				}

				if (p.y < minY) {
					minY = p.y;
				}

				if (p.y > maxY) {
					maxY = p.y;
				}

				lookingAtOcean = true;
			}
		}
	}

	if (lookingAtOcean) {
		minX = minX < -1.f ? -1.f : minX;
		maxX = maxX > 1.f ? 1.f : maxX;
		minY = minY < -1.f ? -1.f : minY;
		maxY = maxY > 1.f ? 1.f : maxY;

		glm::mat4 mRange(1.f);

		mRange[0][0] = maxX - minX;
		mRange[1][1] = maxY - minY;
		mRange[3][0] = minX;
		mRange[3][1] = minY;

		mProjector = projectorCamera.getInverseVP() * mRange;

		uint32 i = 0;
		for (float y = 0.f; y <= 1.f; ++y) {
			for (float x = 0.f; x <= 1.f; ++x) {
				const glm::vec4 a = mProjector * glm::vec4(x, y, -1.f, 1.f);
				const glm::vec4 b = mProjector * glm::vec4(x, y, 1.f, 1.f);
				const glm::vec4 v = b - a;
				const float t = (a.w * ocean.getOceanHeight() - a.y)
					/ (v.y - v.w * ocean.getOceanHeight());

				corners[i++] = a + v * t;
			}
		}
	}
}

const int OceanProjector::cameraEdges[] = {0, 1,  0, 2,  2, 3,  1, 3,
										   0, 4,  2, 6,  3, 7,  1, 5,
										   4, 6,  4, 5,  5, 7,  6, 7};
