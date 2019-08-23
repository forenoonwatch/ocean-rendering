#include "ocean-projector.hpp"

#include "ocean.hpp"

#include <cfloat>
#include <GLM/gtc/matrix_transform.hpp>

OceanProjector::OceanProjector(Ocean& ocean, Camera& viewCamera)
		: ocean(ocean)
		, viewCamera(viewCamera)
		, projectorCamera(viewCamera.getFieldOfView(), viewCamera.getAspectRatio(),
				viewCamera.getZNear(), viewCamera.getZFar())
		, mProjector(1.f) {}

void OceanProjector::update() {
	glm::vec3 camPos = viewCamera.getPosition()
			+ viewCamera.getLookVector() * 2.f * ocean.getMaxAmplitude();

	if (camPos.y < ocean.getOceanHeight() + ocean.getMaxAmplitude()) {
		camPos.y = ocean.getOceanHeight() + ocean.getMaxAmplitude();
	}

	projectorCamera.setPosition(camPos);
	projectorCamera.setRotation(viewCamera.getRotationX(), viewCamera.getRotationY());
	projectorCamera.update();

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
