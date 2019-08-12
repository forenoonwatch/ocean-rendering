#include "camera.hpp"

#include <GLM/gtc/matrix_transform.hpp>

Camera::Camera(float fieldOfView, float aspectRatio, float zNear, float zFar)
		: projection(glm::perspective(fieldOfView, aspectRatio, zNear, zFar))
		, view(1.f)
		, viewProjection(1.f)
		, iViewProjection(1.f)
		, position(0.f, 0.f, 0.f)
		, rotationX(0.f)
		, rotationY(0.f)
		, fieldOfView(fieldOfView)
		, aspectRatio(aspectRatio)
		, zNear(zNear)
		, zFar(zFar) {}

void Camera::move(float dx, float dy, float dz) {
	glm::mat4 rot = glm::rotate(glm::mat4(1.f), rotationY, glm::vec3(0.f, 1.f, 0.f));

	glm::vec3 fwd(rot[2][0], rot[2][1], rot[2][2]);
	glm::vec3 rht(rot[0][0], rot[0][1], rot[0][2]);

	position += rht * dx + fwd * dz;
	position.y += dy;
}

void Camera::rotate(float dx, float dy) {
	rotationX += dx;
	rotationY += dy;

	if (rotationX < -1.27f) {
		rotationX = -1.27f;
	}
	else if (rotationX > 1.27f) {
		rotationX = 1.27f;
	}
}

void Camera::update() {
	view = glm::rotate(glm::mat4(1.f), rotationY, glm::vec3(0.f, 1.f, 0.f));
	view *= glm::rotate(glm::mat4(1.f), rotationX, glm::vec3(1.f, 0.f, 0.f));
	view = glm::translate(glm::mat4(1.f), position) * view;

	viewProjection = projection * glm::inverse(view);
	iViewProjection = glm::inverse(viewProjection);
}

void Camera::update(const Camera& target) {
	position = target.position;
	rotationX = target.rotationX;
	rotationY = target.rotationY;

	view = target.view;
	viewProjection = projection * glm::inverse(view);
	iViewProjection = glm::inverse(viewProjection);
}
