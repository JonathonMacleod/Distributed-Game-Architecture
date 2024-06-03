#ifndef SERVER__SHADERS__H
	#define SERVER__SHADERS__H
	
	#include <vector>
	#include <string>

	#include "Graphics.h"

	namespace Client::Graphics {
		
		class ShaderProgram {
			public:
				ShaderProgram(const std::string& vertexShaderFilepath, const std::string& fragmentShaderFilepath);
				~ShaderProgram();

				void Bind() const;
				void Unbind() const;

				bool HadErrorBuilding() const { return m_HadErrorBuilding; }

				void SetUniformFloatArray(GLint uniformLocation, GLsizei count, const float* data) const;

			private:
				bool m_HadErrorBuilding = false;
				
				GLuint m_ShaderProgramId = 0;
				std::vector<GLuint> m_ShaderProgramIds;

				GLuint CompileShaderSource(const std::string& shaderSource, GLenum shaderType);
		};

	}

#endif