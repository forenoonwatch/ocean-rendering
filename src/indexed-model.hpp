#pragma once

#include "common.hpp"

#include <vector>

class IndexedModel {
	public:
		inline IndexedModel()
			: instancedElementStartIndex((uint32)-1) {}

		void initStaticMesh();

		inline void allocateElement(uint32 elementSize);
		inline void setInstancedElementStartIndex(uint32 elementIndex);

		void addElement1f(uint32 elementIndex, float e0);
		void addElement2f(uint32 elementIndex, float e0, float e1);
		void addElement3f(uint32 elementIndex, float e0, float e1, float e2);
		void addElement4f(uint32 elementIndex, float e0, float e1, float e2, float e3);

		void setElement4f(uint32 elementIndex, uint32 arrayIndex,
				float e0, float e1, float e2, float e3);

		void addIndices1i(uint32 i0);
		void addIndices2i(uint32 i0, uint32 i1);
		void addIndices3i(uint32 i0, uint32 i1, uint32 i2);
		void addIndices4i(uint32 i0, uint32 i1, uint32 i2, uint32 i3);

		inline std::vector<const float*> getVertexData() const;
		inline const uint32* getIndices() const;
		inline const uint32* getElementSizes() const;

		inline uint32 getNumVertexComponents() const;
		inline uint32 getNumInstanceComponents() const;

		inline uint32 getNumVertices() const;
		inline uint32 getNumIndices() const;
		inline uint32 getInstancedElementStartIndex() const;

		inline float getElement(uint32 elementIndex, uint32 arrayIndex) const;
	private:
		std::vector<uint32> indices;
		std::vector<uint32> elementSizes;
		std::vector<std::vector<float>> elements;

		uint32 instancedElementStartIndex;
};

#include "indexed-model.inl"
