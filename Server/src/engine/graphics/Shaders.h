#ifndef SERVER__SHADERS__H
	#define SERVER__SHADERS__H
	
	#include <vector>
	#include <string>

	#include <glm/glm.hpp>

	#include "Graphics.h"

	namespace Server::Engine::Graphics {
		
		class ShaderProgram {
			public:
				ShaderProgram(const std::string& vertexShaderFilepath, const std::string& fragmentShaderFilepath);
				~ShaderProgram();

				void Bind() const;
				void Unbind() const;

				bool HadErrorBuilding() const { return m_HadErrorBuilding; }

				void SetUniformMatrix4(GLint uniformLocation, const glm::mat4& matrix) const;
				void SetUniformFloatArray(GLint uniformLocation, GLsizei count, const float* data) const;
				void SetUniformVec4Array(GLint uniformLocation, const std::vector<glm::vec4>& vectors) const;
				void SetUniformMatrix4Array(GLint uniformLocation, const std::vector<glm::mat4>& matrices) const;

			private:
				bool m_HadErrorBuilding = false;
				
				GLuint m_ShaderProgramId = 0;
				std::vector<GLuint> m_ShaderProgramIds;

				GLuint CompileShaderSource(const std::string& shaderSource, GLenum shaderType);
		};

	}

#endif