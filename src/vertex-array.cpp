#include "vertex-array.hpp"

#include <cstring>

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, uint32 usage)
		: context(&context)
		, arrayID(0)
		, numBuffers(model.getNumVertexComponents() + model.getNumInstanceComponents() + 1)
		, numElements(model.getNumIndices())
		, instancedComponentStartIndex(model.getInstancedElementStartIndex())
		, numOwnedBuffers(numBuffers)
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(usage)
		, indexed(indexed) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	glGenBuffers(numBuffers, buffers);

	std::vector<const float*> vertexData = model.getVertexData();
	initVertexBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes(), true);

	uintptr indicesSize = numElements * sizeof(uint32);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize,
			model.getIndices(), usage);

	bufferSizes[numBuffers - 1] = indicesSize;
}

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, VertexArray& vertexArray)
		: context(&context)
		, arrayID(0)
		, numBuffers(vertexArray.numBuffers)
		, numElements(vertexArray.numElements)
		, instancedComponentStartIndex(vertexArray.instancedComponentStartIndex)
		, numOwnedBuffers(model.getNumInstanceComponents())
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(vertexArray.usage)
		, indexed(vertexArray.indexed) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	std::memcpy(buffers, vertexArray.buffers, numBuffers * sizeof(GLuint));

	glGenBuffers(numOwnedBuffers, buffers + instancedComponentStartIndex);

	std::vector<const float*> vertexData = model.getVertexData();
	initVertexBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes(), false);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	
	bufferSizes[numBuffers - 1] = vertexArray.bufferSizes[numBuffers - 1];
}

VertexArray::VertexArray(RenderContext& context, uint32 numBuffers,
			const uint32* elementSizes, uint32 numElements, uint32 usage)
		: context(&context)
		, arrayID(0)
		, numBuffers(numBuffers)
		, numElements(numElements)
		, instancedComponentStartIndex((uint32)-1)
		, numOwnedBuffers(numBuffers)
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(usage)
		, indexed(false) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	glGenBuffers(numBuffers, buffers);

	initEmptyArrayBuffers(numBuffers, numElements, elementSizes);
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
	if (numBuffers == numOwnedBuffers) {
		glDeleteBuffers(numBuffers, buffers);
	}
	else {
		glDeleteBuffers(numOwnedBuffers, buffers + instancedComponentStartIndex);
	}
	
	glDeleteVertexArrays(1, &arrayID);
	context->setVertexArray(0);

	delete[] buffers;
	delete[] bufferSizes;
}

void VertexArray::initVertexBuffers(uint32 numVertexComponents,
		const float** vertexData, uint32 numVertices, const uint32* vertexElementSizes,
		bool writeData) {
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

		if (writeData || instancedMode) {
			glBufferData(GL_ARRAY_BUFFER, dataSize, bufferData, attribUsage);
		}
		
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

void VertexArray::initEmptyArrayBuffers(uint32 numVertexComponents,
		uint32 numVertices, const uint32* vertexElementSizes) {
	for (uint32 i = 0, attribute = 0; i < numBuffers; ++i) {
		uint32 attribUsage = usage;
		bool instancedMode = false;

		if (i >= numVertexComponents) {
			attribUsage = GL_DYNAMIC_DRAW;
			instancedMode = true;
		}

		uint32 elementSize = vertexElementSizes[i];
		uintptr dataSize = instancedMode ? elementSize * sizeof(float)
			: elementSize * sizeof(float) * numVertices;

		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
		glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, attribUsage);

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
