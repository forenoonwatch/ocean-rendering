#include "render-query.hpp"

#include <GL/glew.h>

RenderQuery::RenderQuery(uint32 queryType)
		: queryType(queryType) {
	glGenQueries(1, &queryID);
}

uint32 RenderQuery::getResultInt() {
	uint32 res;
	glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &res);

	return res;
}

RenderQuery::~RenderQuery() {
	glDeleteQueries(1, &queryID);
}
