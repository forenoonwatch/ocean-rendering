#pragma once

#include <cstdint>
#include <vector>

class IndexedModel {
	public:
		inline IndexedModel()
			: instancedElementStartIndex((uint32_t)-1) {}

		inline void allocateElement(uint32_t elementSize);
		inline void setInstancedElementStartIndex(uint32_t elementIndex);

		void addElement1f(uint32_t elementIndex, float e0);
		void addElement2f(uint32_t elementIndex, float e0, float e1);
		void addElement3f(uint32_t elementIndex, float e0, float e1, float e2);
		void addElement4f(uint32_t elementIndex, float e0, float e1, float e2, float e3);

		void setElement4f(uint32_t elementIndex, uint32_t arrayIndex,
				float e0, float e1, float e2, float e3);

		void addIndices1i(uint32_t i0);
		void addIndices2i(uint32_t i0, uint32_t i1);
		void addIndices3i(uint32_t i0, uint32_t i1, uint32_t i2);
		void addIndices4i(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3);

		inline const float** getVertexData() const;
		inline const uint32_t* getIndices() const;
		inline const uint32_t* getElementSizes() const;

		inline uint32_t getNumVertexComponents() const;
		inline uint32_t getNumInstanceComponents() const;

		inline uint32_t getNumVertices() const;
		inline uint32_t getNumIndices() const;
		inline uint32_t getInstancedElementStartIndex() const;

		inline float getElement(uint32_t elementIndex, uint32_t arrayIndex) const;
	private:
		std::vector<uint32_t> indices;
		std::vector<uint32_t> elementSizes;
		std::vector<std::vector<float>> elements;

		uint32_t instancedElementStartIndex;
};

#include "indexed-model.inl"
