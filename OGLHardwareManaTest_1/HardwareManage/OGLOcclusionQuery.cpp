#include "OGLOcclusionQuery.h"

namespace Lean3D
{
	uint32 OGLOcclusionQuery::createOcclusionQuery()
	{
		uint32 queryRef;
		glGenQueries(1, &queryRef);
		
		return queryRef;
	}

	void OGLOcclusionQuery::destroyQuery(uint32 queryRef)
	{
		if (queryRef == 0) return;

		glDeleteQueries(1, &queryRef);
	}

	void OGLOcclusionQuery::beginQuery(uint32 queryRef)
	{
		glBeginQuery(GL_SAMPLES_PASSED, queryRef);
	}

	void OGLOcclusionQuery::endQuery(uint32 queryRef)
	{
		glEndQuery(GL_SAMPLES_PASSED);
	}

	uint32 OGLOcclusionQuery::getQueryResult(uint32 queryRef)
	{
		uint32 samples = 0;
		
		glGetQueryObjectuiv(queryRef, GL_QUERY_RESULT, &samples);

		return samples;
	}

}