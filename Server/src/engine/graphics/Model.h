#ifndef SERVER__MODEL__H
	#define SERVER__MODEL__H

	#include <vector>

	#include "Graphics.h"

	namespace Server::Engine::Graphics {

		class Model {
			public:
				Model();
				~Model();

				void Set2DPositionsBuffer(const float* positionsArray, GLsizeiptr arraySize);
				void Set3DPositionsBuffer(const float* positionsArray, GLsizeiptr arraySize);
				void SetIndexBuffer(const unsigned int* indexBufferArray, GLsizeiptr arraySize);
				void SetColoursBuffer(const float* coloursArray, GLsizeiptr arraySize);
				void SetTextureCoordinatesBuffer(const float* textureCoordinatesArray, GLsizeiptr arraySize);

				void DrawIndividual() const;
				void DrawInstanced(GLsizei count) const;

				bool HasTextureCoordinates() const { return m_HasTextureCoordinates; }

			private:
				bool m_HasTextureCoordinates = false;

				GLuint m_VertexArrayObjectId = 0;
				GLsizei m_NumberOfVertices = 0;

				bool m_HasIndexBufferObject = false;
				GLuint m_IndexBufferObject = 0;
				GLsizei m_NumberOfIndices = 0;

				std::vector<GLuint> m_VertexBufferObjects;
		};

	}

#endif