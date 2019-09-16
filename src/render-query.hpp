#pragma once

#include "common.hpp"

class RenderQuery {
	public:
		RenderQuery(uint32 queryType);

		uint32 getResultInt();

		inline uint32 getID() { return queryID; }

		inline uint32 getType() const { return queryType; }

		~RenderQuery();
	private:
		NULL_COPY_AND_ASSIGN(RenderQuery);

		uint32 queryID;
		uint32 queryType;
};
