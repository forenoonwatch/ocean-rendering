
inline void IndexedModel::allocateElement(uint32 elementSize) {
	elementSizes.push_back(elementSize);
	elements.emplace_back();
}

inline void IndexedModel::setInstancedElementStartIndex(uint32 elementIndex) {
	instancedElementStartIndex = elementIndex;
}

inline std::vector<const float*> IndexedModel::getVertexData() const {
	std::vector<const float*> vertexData;

	for (uint32 i = 0; i < getNumVertexComponents(); ++i) {
		vertexData.push_back(&elements[i][0]);
	}

	return vertexData;
}

inline const uint32* IndexedModel::getIndices() const {
	return &indices[0];
}

inline const uint32* IndexedModel::getElementSizes() const {
	return &elementSizes[0];
}

inline uint32 IndexedModel::getNumVertexComponents() const {
	return elementSizes.size() - getNumInstanceComponents();
}

inline uint32 IndexedModel::getNumInstanceComponents() const {
	return instancedElementStartIndex == ((uint32)-1)
		? 0 : (elementSizes.size() - instancedElementStartIndex);
}

inline uint32 IndexedModel::getNumVertices() const {
	return elements[0].size() / elementSizes[0];
}

inline uint32 IndexedModel::getNumIndices() const {
	return indices.size();
}

inline uint32 IndexedModel::getInstancedElementStartIndex() const {
	return instancedElementStartIndex;
}

inline float IndexedModel::getElement(uint32 elementIndex,
		uint32 arrayIndex) const {
	return elements[elementIndex][arrayIndex];
}
