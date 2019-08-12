
#include <cstdio>

inline void IndexedModel::allocateElement(uint32_t elementSize) {
	elementSizes.push_back(elementSize);
	elements.emplace_back();
}

inline void IndexedModel::setInstancedElementStartIndex(uint32_t elementIndex) {
	instancedElementStartIndex = elementIndex;
}

inline std::vector<const float*> IndexedModel::getVertexData() const {
	std::vector<const float*> vertexData;

	for (uint32_t i = 0; i < getNumVertexComponents(); ++i) {
		vertexData.push_back(&elements[i][0]);
	}

	return vertexData;
}

inline const uint32_t* IndexedModel::getIndices() const {
	return &indices[0];
}

inline const uint32_t* IndexedModel::getElementSizes() const {
	return &elementSizes[0];
}

inline uint32_t IndexedModel::getNumVertexComponents() const {
	return elementSizes.size() - getNumInstanceComponents();
}

inline uint32_t IndexedModel::getNumInstanceComponents() const {
	return instancedElementStartIndex == ((uint32_t)-1)
		? 0 : (elementSizes.size() - instancedElementStartIndex);
}

inline uint32_t IndexedModel::getNumVertices() const {
	return elements[0].size() / elementSizes[0];
}

inline uint32_t IndexedModel::getNumIndices() const {
	return indices.size();
}

inline uint32_t IndexedModel::getInstancedElementStartIndex() const {
	return instancedElementStartIndex;
}

inline float IndexedModel::getElement(uint32_t elementIndex,
		uint32_t arrayIndex) const {
	return elements[elementIndex][arrayIndex];
}
