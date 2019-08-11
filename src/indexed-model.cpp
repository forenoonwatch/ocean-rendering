#include "indexed-model.hpp"

void IndexedModel::addElement1f(uint32_t elementIndex, float e0) {
	elements[elementIndex].push_back(e0);
}

void IndexedModel::addElement2f(uint32_t elementIndex, float e0, float e1) {
	elements[elementIndex].push_back(e0);
	elements[elementIndex].push_back(e1);
}

void IndexedModel::addElement3f(uint32_t elementIndex, float e0, float e1, float e2) {
	elements[elementIndex].push_back(e0);
	elements[elementIndex].push_back(e1);
	elements[elementIndex].push_back(e2);
}

void IndexedModel::addElement4f(uint32_t elementIndex, float e0, float e1, float e2, float e3) {
	elements[elementIndex].push_back(e0);
	elements[elementIndex].push_back(e1);
	elements[elementIndex].push_back(e2);
	elements[elementIndex].push_back(e3);
}

void IndexedModel::setElement4f(uint32_t elementIndex, uint32_t arrayIndex,
		float e0, float e1, float e2, float e3) {
	arrayIndex *= elementSizes[elementIndex];

	elements[elementIndex][arrayIndex] = e0;
	elements[elementIndex][arrayIndex + 1] = e1;
	elements[elementIndex][arrayIndex + 2] = e2;
	elements[elementIndex][arrayIndex + 3] = e3;
}

void IndexedModel::addIndices1i(uint32_t i0) {
	indices.push_back(i0);
}

void IndexedModel::addIndices2i(uint32_t i0, uint32_t i1) {
	indices.push_back(i0);
	indices.push_back(i1);
}

void IndexedModel::addIndices3i(uint32_t i0, uint32_t i1, uint32_t i2) {
	indices.push_back(i0);
	indices.push_back(i1);
	indices.push_back(i2);
}

void IndexedModel::addIndices4i(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) {
	indices.push_back(i0);
	indices.push_back(i1);
	indices.push_back(i2);
	indices.push_back(i3);
}
