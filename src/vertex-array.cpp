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
		, indexed(indexed)
		, bufferOwnership(FULLY_OWNED) {
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
		, indexed(vertexArray.indexed)
		, bufferOwnership(SHARED_VERTEX_BUFFERS) {
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
		, indexed(false)
		, bufferOwnership(FULLY_OWNED) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	glGenBuffers(numBuffers, buffers);

	initEmptyArrayBuffers(numBuffers, numElements, elementSizes);
}

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, TransformFeedback& tfb,
			uint32 bufferNum, uint32 usage)
		: context(&context)
		, arrayID(0)
		, numBuffers(model.getNumVertexComponents() + 2)
		, numElements(model.getNumIndices())
		, instancedComponentStartIndex(model.getInstancedElementStartIndex())
		, numOwnedBuffers(numBuffers - 1)
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(usage)
		, indexed(true)
		, bufferOwnership(SHARED_INSTANCE_BUFFERS) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	glGenBuffers(model.getNumVertexComponents(), buffers);
	glGenBuffers(1, &buffers[numBuffers - 1]);

	buffers[numBuffers - 2] = tfb.getBuffer(bufferNum);

	std::vector<const float*> vertexData = model.getVertexData();
	initSharedBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes(),
			tfb.getNumAttribs(), tfb.getAttribSizes(),
			tfb.getDataBlockSize(), tfb.getBufferSize(), true);

	uintptr indicesSize = numElements * sizeof(uint32);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize,
			model.getIndices(), usage);

	bufferSizes[numBuffers - 1] = indicesSize;
}

VertexArray::VertexArray(RenderContext& context,
			const IndexedModel& model, VertexArray& vertexArray,
			TransformFeedback& tfb, uint32 bufferNum)
		: context(&context)
		, arrayID(0)
		, numBuffers(model.getNumVertexComponents() + 2)
		, numElements(vertexArray.numElements)
		, instancedComponentStartIndex(vertexArray.instancedComponentStartIndex)
		, numOwnedBuffers(0)
		, buffers(new GLuint[numBuffers])
		, bufferSizes(new uintptr[numBuffers])
		, usage(vertexArray.usage)
		, indexed(vertexArray.indexed)
		, bufferOwnership(FULLY_SHARED) {
	glGenVertexArrays(1, &arrayID);
	context.setVertexArray(arrayID);

	std::memcpy(buffers, vertexArray.buffers,
			model.getNumVertexComponents() * sizeof(GLuint));

	buffers[numBuffers - 2] = tfb.getBuffer(bufferNum);

	std::vector<const float*> vertexData = model.getVertexData();
	initSharedBuffers(model.getNumVertexComponents(), &vertexData[0],
			model.getNumVertices(), model.getElementSizes(),
			tfb.getNumAttribs(), tfb.getAttribSizes(),
			tfb.getDataBlockSize(), tfb.getBufferSize(), false);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
	
	bufferSizes[numBuffers - 1] = vertexArray.bufferSizes[numBuffers - 1];
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
	switch (bufferOwnership) {
		case FULLY_OWNED:
			glDeleteBuffers(numBuffers, buffers);
			break;
		case SHARED_VERTEX_BUFFERS:
			glDeleteBuffers(numOwnedBuffers,
					buffers + instancedComponentStartIndex);
			break;
		case SHARED_INSTANCE_BUFFERS:
			glDeleteBuffers(numOwnedBuffers, buffers);
			
			if (indexed) {
				glDeleteBuffers(1, &buffers[numBuffers - 1]);
			}
			break;
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

void VertexArray::initSharedBuffers(uint32 numVertexComponents,
		const float** vertexData, uint32 numVertices, const uint32* vertexElementSizes,
		uint32 numInstanceComponents, const uint32* instanceElementSizes,
		uint32 instanceDataSize, uint32 instanceBufferSize, bool writeVertexComponents) {
	uint32 attribute = 0;

	for (uint32 i = 0; i < numVertexComponents; ++i) {
		const uint32 elementSize = vertexElementSizes[i];
		const uintptr dataSize = elementSize * sizeof(float) * numVertices;

		glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

		if (writeVertexComponents) {
			glBufferData(GL_ARRAY_BUFFER, dataSize, vertexData[i], usage);
		}

		bufferSizes[i] = dataSize;

		uint32 elementSizeDiv = elementSize / 4;
		uint32 elementSizeRem = elementSize % 4;

		for (uint32 j = 0; j < elementSizeDiv; ++j) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE,
					elementSize * sizeof(float),
					(const void*)(j * 4 * sizeof(float)));

			++attribute;
		}

		if (elementSizeRem != 0) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, elementSize, GL_FLOAT, GL_FALSE,
					elementSize * sizeof(float),
					(const void*)(elementSizeDiv * 4 * sizeof(float)));

			++attribute;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffers[numVertexComponents]);
	bufferSizes[numVertexComponents] = instanceBufferSize; 

	for (uint32 i = 0, offset = 0; i < numInstanceComponents; ++i) {
		const uint32 elementSize = instanceElementSizes[i];

		uint32 elementSizeDiv = elementSize / 4;
		uint32 elementSizeRem = elementSize % 4;

		for (uint32 j = 0; j < elementSizeDiv; ++j) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE,
					instanceDataSize, (const void*)(offset));

			offset += 4 * sizeof(float);
			++attribute;
		}

		if (elementSizeRem != 0) {
			glEnableVertexAttribArray(attribute);
			glVertexAttribPointer(attribute, elementSizeRem, GL_FLOAT, GL_FALSE,
					instanceDataSize, (const void*)(offset));

			offset += elementSizeRem * sizeof(float);
			++attribute;
		}
	}
}
