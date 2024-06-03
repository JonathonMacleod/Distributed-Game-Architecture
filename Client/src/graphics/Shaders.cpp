#include "Shaders.h"

#include "Middleware.h"

namespace Client::Graphics {

	ShaderProgram::ShaderProgram(const std::string& vertexShaderFilepath, const std::string& fragmentShaderFilepath) {
		// Create a shader program
		m_ShaderProgramId = glCreateProgram();

		// Compile and attach vertex shader to shader program
		const std::string vertexShaderFileContents = Middleware::Utils::ReadFileContents(vertexShaderFilepath);
		const GLuint vertexShader = CompileShaderSource(vertexShaderFileContents, GL_VERTEX_SHADER);
		if(vertexShader == 0) {
			CLIENT_ERROR("Failed to compile vertex shader");
			m_HadErrorBuilding = true;
			return;
		}
		glAttachShader(m_ShaderProgramId, vertexShader);

		// Compile and attach fragment shader to shader program
		const std::string fragmentShaderFileContents = Middleware::Utils::ReadFileContents(fragmentShaderFilepath);
		const GLuint fragmentShader = CompileShaderSource(fragmentShaderFileContents, GL_FRAGMENT_SHADER);
		if(fragmentShader == 0) {
			SERVER_ERROR("Failed to compile fragment shader");
			m_HadErrorBuilding = true;
			return;
		}
		glAttachShader(m_ShaderProgramId, fragmentShader);

		// Link the shader program
		glLinkProgram(m_ShaderProgramId);

		// Retrieve log information about linking the shader program
		GLint shaderProgramLogLength = 0;
		glGetProgramiv(m_ShaderProgramId, GL_INFO_LOG_LENGTH, &shaderProgramLogLength);
		std::vector<GLchar> shaderProgramLog;
		if(shaderProgramLogLength > 0) {
			shaderProgramLog.resize(shaderProgramLogLength);
			glGetProgramInfoLog(m_ShaderProgramId, GL_INFO_LOG_LENGTH, nullptr, shaderProgramLog.data());
		}

		// Check for shader program linking errors
		GLint linkingStatus = 0;
		glGetProgramiv(m_ShaderProgramId, GL_LINK_STATUS, &linkingStatus);
		if(linkingStatus != GL_TRUE) {
			CLIENT_ERROR("Failed to link shader program. Error: %s", shaderProgramLog.data());
			m_HadErrorBuilding = true;
			return;
		}
	}

	ShaderProgram::~ShaderProgram() { 
		glDeleteProgram(m_ShaderProgramId); 
	}

	GLuint ShaderProgram::CompileShaderSource(const std::string& shaderSource, GLenum shaderType) {
		// Generate an ID for this shader
		GLuint shaderId = glCreateShader(shaderType);

		// OpenGL expects a C-style string (although GLchar* is specified). This is just to loose the const-ness of the std::string data (by making a local copy)
		std::vector<GLchar> sourceChars(shaderSource.size());
		for(size_t i = 0; i < shaderSource.length(); i++) {
			sourceChars[i] = ((GLchar) shaderSource[i]);
		}

		// Pass OpenGL the source code
		GLchar* sources[] = { sourceChars.data() };
		GLsizei sourceLengths[] = { GLsizei(shaderSource.length()) };
		glShaderSource(shaderId, 1, sources, sourceLengths);

		// Compile the code
		glCompileShader(shaderId);

		// Retrieve log information about the shaders
		GLint shaderLogLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &shaderLogLength);
		std::vector<GLchar> shaderLog;
		if(shaderLogLength > 0) {
			shaderLog.resize(shaderLogLength);
			glGetShaderInfoLog(shaderId, GL_INFO_LOG_LENGTH, nullptr, shaderLog.data());
		}

		// Check for compilation errors
		GLint compilationStatus = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
		if(compilationStatus != GL_TRUE) {
			glDeleteShader(shaderId);
			CLIENT_ERROR("Failed to compile shader source. Error: %s", shaderLog.data());
			return 0;
		}

		return shaderId;
	}

	void ShaderProgram::Bind() const { 
		glUseProgram(m_ShaderProgramId); 
	}

	void ShaderProgram::Unbind() const { 
		glUseProgram(0); 
	}

	void ShaderProgram::SetUniformFloatArray(GLint uniformLocation, GLsizei count, const float* data) const {
		glUniform1fv(uniformLocation, count, data);
	}

}