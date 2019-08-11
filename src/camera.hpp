#pragma once

#include <GLM/glm.hpp>

class Camera {
	public:
		Camera(float fieldOfView, float aspectRatio, float zNear, float zFar);

		void move(float dx, float dy, float dz);
		void rotate(float dx, float dy);
		void update();
		void update(const Camera&);

		inline const glm::mat4& getProjection() const { return projection; }
		inline const glm::mat4& getView() const { return view; }
		inline const glm::mat4& getViewProjection() const { return viewProjection; }
		inline const glm::mat4& getInverseVP() const { return iViewProjection; }
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
