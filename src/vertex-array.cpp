#include "vertex-array.hpp"

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, uint32 usage)
		: context(&context)
		, arrayID(0)
		, numBuffers(model.getNumVertexComponents() + model.getNumInstanceComponents() + 1)
		, numIndices(model.getNumIndices())
		, instancedComponentStartIndex(model.getInstancedElementStartIndex())
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(usage) {
	glGenVertexArrays(1, &arrayID);
	glBindVertexArray(arrayID);

	glGenBuffers(numBuffers, buffers);

	std::vector<const float*> vertexData = model.getVertexData();
	initVertexBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes());

	uintptr indicesSize = numIndices * sizeof(uint32);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize,
			model.getIndices(), usage);

	bufferSizes[numBuffers - 1] = indicesSize;
}

void VertexArray::updateBuffer(uint32 bufferIndex,
		const void* data, uintptr dataSize) {
	context->setVertexArray(arrayID);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[bufferIndex]);

	if (dataSize <= bufferSizes[bufferIndex]) {
		glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, dataSize, data,
				bufferIndex >= instancedComponentStartIndex ? GL_DYNAMIC_DRAW : usage);
	}
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &arrayID);
	glDeleteBuffers(numBuffers, buffers);

	delete[] buffers;
	delete[] bufferSizes;
}

inline void VertexArray::initVertexBuffers(uint32 numVertexComponents,
		const float** vertexData, uint32 numVertices, const uint32* vertexElementSizes) {
	for (uint32 i = 0, attribute = 0; i < numBuffers - 1; ++i) {
		uint32 attribUsage = usage;
		bool instancedMode = false;

		if (i >= numVertexComponents) {
			attribUsage = GL_DYNAMIC_DRAW;
			instancedMode = true;
		}

		uint32 elementSize = vertexElementSizes[i];
		const void* bufferData = instancedMode ? nullptr : vertexData[i];
		uintptr dataSize = instancedMode ? elementSize * sizeof(float)
			: elementSize * sizeof(float) * numVertices;

		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
		glBufferData(GL_ARRAY_BUFFER, dataSize, bufferData, attribUsage);

		bufferSizes[i] = dataSize;

		uint32 elementSizeDiv = elementSize / 4;
		uint32 elementSizeRem = elementSize % 4;

		for (uint32 j = 0; j < elementSizeDiv; ++j) {
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
