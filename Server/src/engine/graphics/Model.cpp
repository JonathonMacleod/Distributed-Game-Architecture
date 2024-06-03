#include "Model.h"

namespace Server::Engine::Graphics {

	Model::Model() {
		glGenVertexArrays(1, &m_VertexArrayObjectId);
	}

	Model::~Model() {
		// Destroy all the buffered data and then delete the vertex array that held them all
		for(size_t i = 0; i < m_VertexBufferObjects.size(); i++) {
			glDeleteBuffers(1, &m_VertexBufferObjects.at(i));
		}
		glDeleteVertexArrays(1, &m_VertexArrayObjectId);
	}

	void Model::Set2DPositionsBuffer(const float* positionsArray, GLsizeiptr arraySize) {
		glBindVertexArray(m_VertexArrayObjectId);

		// Generate a buffer with the vertex data and bind it
		GLuint positionsBuffer = 0;
		glGenBuffers(1, &positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);

		// Put data into the buffer
		glBufferData(GL_ARRAY_BUFFER, arraySize, positionsArray, GL_STATIC_DRAW);

		// Bind the positions buffer object to attribute 0 in the vertex array object
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Store the number of positions for later drawing
		m_NumberOfVertices = ((GLsizei) arraySize) / ((GLsizei) sizeof(float));

		// Cleanup (unbind the vertex array and buffer)
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_VertexBufferObjects.push_back(positionsBuffer);
	}

	void Model::Set3DPositionsBuffer(const float* positionsArray, GLsizeiptr arraySize) {
		glBindVertexArray(m_VertexArrayObjectId);

		// Generate a buffer with the vertex data and bind it
		GLuint positionsBuffer = 0;
		glGenBuffers(1, &positionsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);

		// Put data into the buffer
		glBufferData(GL_ARRAY_BUFFER, arraySize, positionsArray, GL_STATIC_DRAW);

		// Bind the positions buffer object to attribute 0 in the vertex array object
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Store the number of positions for later drawing
		m_NumberOfVertices = ((GLsizei) arraySize) / ((GLsizei) sizeof(float));

		// Cleanup (unbind the vertex array and buffer)
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_VertexBufferObjects.push_back(positionsBuffer);
	}

	void Model::SetIndexBuffer(const unsigned int* indexBufferArray, GLsizeiptr arraySize) {
		m_HasIndexBufferObject = true;
		m_NumberOfIndices = ((GLsizei) arraySize) / ((GLsizei) sizeof(unsigned int));

		// Bind the vertex array
		glBindVertexArray(m_VertexArrayObjectId);

		// Generate an index buffer and bind it
		glGenBuffers(1, &m_IndexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);

		// Put data into the buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, arraySize, indexBufferArray, GL_STATIC_DRAW);

		// Unbind the index buffer and the vertex array
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Model::SetColoursBuffer(const float* coloursArray, GLsizeiptr arraySize) {
		glBindVertexArray(m_VertexArrayObjectId);

		// Generate a buffer with the colours data and bind it
		GLuint coloursBuffer = 0;
		glGenBuffers(1, &coloursBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, coloursBuffer);

		// Put data into the buffer
		glBufferData(GL_ARRAY_BUFFER, arraySize, coloursArray, GL_STATIC_DRAW);

		// Bind the colours buffer object to attribute 1 in the vertex array object
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Cleanup (unbind the vertex array and buffer)
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_VertexBufferObjects.push_back(coloursBuffer);
	}

	void Model::SetTextureCoordinatesBuffer(const float* textureCoordinatesArray, GLsizeiptr arraySize) {
		m_HasTextureCoordinates = true;

		glBindVertexArray(m_VertexArrayObjectId);

		// Generate a buffer with the texture coordinate data and bind it
		GLuint texturesBuffer = 0;
		glGenBuffers(1, &texturesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, texturesBuffer);

		// Put data into the buffer
		glBufferData(GL_ARRAY_BUFFER, arraySize, textureCoordinatesArray, GL_STATIC_DRAW);

		// Bind the colours buffer object to attribute 1 in the vertex array object
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// Cleanup (unbind the vertex array and buffer)
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_VertexBufferObjects.push_back(texturesBuffer);
	}

	void Model::DrawIndividual() const {
		// Bind the vertex array
		glBindVertexArray(m_VertexArrayObjectId);

		if(m_HasIndexBufferObject) {
			// If an index buffer has been provided, bind the index buffer and then draw the elements provided
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);
			glDrawElements(GL_TRIANGLES, m_NumberOfIndices, GL_UNSIGNED_INT, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		} else {
			// If no index buffer was provided, draw using the bound vertex array only
			glDrawArrays(GL_TRIANGLES, 0, m_NumberOfVertices);
		}

		// Unbind the vertex array
		glBindVertexArray(0);
	}

	void Model::DrawInstanced(GLsizei count) const {
		// Bind the vertex array
		glBindVertexArray(m_VertexArrayObjectId);

		if(m_HasIndexBufferObject) {
			// If an index buffer has been provided, bind the index buffer and then draw the elements provided
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);
			glDrawElementsInstanced(GL_TRIANGLES, m_NumberOfIndices, GL_UNSIGNED_INT, nullptr, count);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		} else {
			// If no index buffer was provided, draw using the bound vertex array only
			glDrawArraysInstanced(GL_TRIANGLES, 0, m_NumberOfVertices, count);
		}

		// Unbind the vertex array
		glBindVertexArray(0);
	}

}