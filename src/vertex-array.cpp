#include "vertex-array.hpp"

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, uint32_t usage)
		: context(&context)
		, deviceID(0)
		, numBuffers(model.getNumVertexComponents() + model.getNumInstanceComponents() + 1)
		, numIndices(model.getNumIndices())
		, instancedComponentStartIndex(model.getInstancedElementStartIndex())
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr_t[numBuffers])
		, usage(usage) {
	glGenVertexArrays(1, &deviceID);
	glBindVertexArray(deviceID);

	glGenBuffers(numBuffers, buffers);

	std::vector<const float*> vertexData = model.getVertexData();
	initVertexBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes());

	uintptr_t indicesSize = numIndices * sizeof(uint32_t);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize,
			model.getIndices(), usage);

	bufferSizes[numBuffers - 1] = indicesSize;
}

void VertexArray::updateBuffer(uint32_t bufferIndex,
		const void* data, uintptr_t dataSize) {
	glBindVertexArray(deviceID);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[bufferIndex]);

	if (dataSize <= bufferSizes[bufferIndex]) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, dataSize, data,
				bufferIndex >= instancedComponentStartIndex ? GL_DYNAMIC_DRAW : usage);
	}
}

void VertexArray::draw(uint32_t primitive, uint32_t numInstances) {
	glBindVertexArray(deviceID);

	switch (numInstances) {
		case 0:
			return;
		case 1:
			glDrawElements(primitive, (GLsizei)numIndices, GL_UNSIGNED_INT, 0);
			return;
		default:
			glDrawElementsInstanced(primitive, (GLsizei)numIndices,
					GL_UNSIGNED_INT, 0, numInstances);
	}
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &deviceID);
	glDeleteBuffers(numBuffers, buffers);

	delete[] buffers;
	delete[] bufferSizes;
}

inline void VertexArray::initVertexBuffers(uint32_t numVertexComponents,
		const float** vertexData, uint32_t numVertices, const uint32_t* vertexElementSizes) {
	for (uint32_t i = 0, attribute = 0; i < numBuffers - 1; ++i) {
		uint32_t attribUsage = usage;
		bool instancedMode = false;

		if (i >= numVertexComponents) {
			attribUsage = GL_DYNAMIC_DRAW;
			instancedMode = true;
		}

		uint32_t elementSize = vertexElementSizes[i];
		const void* bufferData = instancedMode ? nullptr : vertexData[i];
		uintptr_t dataSize = instancedMode ? elementSize * sizeof(float)
			: elementSize * sizeof(float) * numVertices;

		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
		glBufferData(GL_ARRAY_BUFFER, dataSize, bufferData, attribUsage);

		bufferSizes[i] = dataSize;

		uint32_t elementSizeDiv = elementSize / 4;
		uint32_t elementSizeRem = elementSize % 4;

		for (uint32_t j = 0; j < elementSizeDiv; ++j) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE,
					elementSize * sizeof(float),
					(const void*)(j * 4 * sizeof(float)));

			if (instancedMode) {
				glVertexAttribDivisor(attribute, 1);
			}

			++attribute;
		}

		if (elementSizeRem != 0) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, elementSize, GL_FLOAT, GL_FALSE,
					elementSize * sizeof(float),
					(const void*)(elementSizeDiv * 4 * sizeof(float)));

			if (instancedMode) {
				glVertexAttribDivisor(attribute, 1);
			}

			++attribute;
		}
	}
}
