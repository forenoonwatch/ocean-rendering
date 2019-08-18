#pragma once

#include <GLM/glm.hpp>

class Camera {
	public:
		Camera(float fieldOfView, float aspectRatio, float zNear, float zFar);

		void move(float dx, float dy, float dz);
		void rotate(float dx, float dy);
		void update();
		void update(const Camera&);

		inline void setPosition(const glm::vec3& position) { this->position = position; }
		inline void setRotation(float rotationX, float rotationY) {
			this->rotationX = rotationX;
			this->rotationY = rotationY;
		}

		glm::mat4 getReflectionVP() const;
		glm::mat4 getReflectionSkybox() const;

		inline const glm::mat4& getProjection() const { return projection; }
		inline const glm::mat4& getView() const { return view; }
		inline const glm::mat4& getViewProjection() const { return viewProjection; }
		inline const glm::mat4& getInverseVP() const { return iViewProjection; }

		inline const glm::vec3& getPosition() const { return position; }
		inline float getRotationX() const { return rotationX; }
		inline float getRotationY() const { return rotationY; }

		inline glm::vec3 getLookVector() const { return glm::vec3(view[2]); }

		inline float getFieldOfView() const { return fieldOfView; }
		inline float getAspectRatio() const { return aspectRatio; }
		inline float getZNear() const { return zNear; }
		inline float getZFar() const { return zFar; }
	private:
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 viewProjection;
		glm::mat4 iViewProjection;

		glm::vec3 position;
		float rotationX;
		float rotationY;

		float fieldOfView;
		float aspectRatio;
		float zNear;
		float zFar;
};
