//
// Created by monty on 23/11/15.
//



#include <GLES2/gl2.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <android/log.h>

#include "NativeBitmap.h"
#include "Texture.h"

#include "GLES2Lesson.h"
#include "NdkGlue.h"

namespace odb {
	const static bool kShouldDestroyThingsManually = false;

	const float GLES2Lesson::billboardVertices[] {
			-1.0f, 1.0f, 0.0f, 0.0f, .0f,
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	};

	const float GLES2Lesson::floorVertices[] {
			-1.0f, 0.0f, -1.0f, 0.0f, .0f,
			1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	};


	const float GLES2Lesson::cubeVertices[]{
//    4________5
//    /|       /|
//   / |      / |
// 0/__|___1_/  |
//  | 7|____|___|6
//  |  /    |  /
//  | /     | /
// 3|/______|/2
//x, y, z, r, g, b, u, v
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f,    //0
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     //1
			1.0f, -1.0f, 1.0f, 1.0f, 1.0f,   //2
			-1.0f, -1.0f, 1.0f, 0.0f, 1.0f,   //3

			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f,   //4
			1.0f, 1.0f, -1.0f, 1.0f, 1.0f,    //5
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f,   //6
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,   //7

			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f,    //8 (0)
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     //9 (1)
			1.0f, -1.0f, 1.0f, 1.0f, 1.0f,   //10 (2)
			-1.0f, -1.0f, 1.0f, 0.0f, 1.0f,   //11 (3)

			-1.0f, 1.0f, -1.0f, 1.0f, 0.0f,   //12 (4)
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f,    //13 (5)
			1.0f, -1.0f, -1.0f, 0.0f, 1.0f,   //14 (6)
			-1.0f, -1.0f, -1.0f, 1.0f, 1.0f   //15 (7)
	};

	const unsigned short GLES2Lesson::billboardIndices[] {
			0, 1, 2,
	        0, 2, 3
	};

	const unsigned short GLES2Lesson::floorIndices[] {
			0, 1, 2,
			0, 2, 3
	};


	const unsigned short GLES2Lesson::cubeIndices[]{
			0, 1, 2,
			0, 2, 3,

			5, 4, 7,
			5, 7, 6,

			9, 13, 14,
			9, 14, 10,

			12, 8, 15,
			8, 11, 15
	};

	unsigned int uploadTextureData(std::shared_ptr<NativeBitmap> bitmap) {
		// Texture object handle
		unsigned int textureId = 0;

		//Generate texture storage
		glGenTextures(1, &textureId);

		//specify what we want for that texture
		glBindTexture(GL_TEXTURE_2D, textureId);

		//upload the data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->getWidth(), bitmap->getHeight(), 0, GL_RGBA,
		             GL_UNSIGNED_BYTE, bitmap->getPixelData());

		// Set the filtering mode - surprisingly, this is needed.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		return textureId;
	}


	extern void printGLString(const char *name, GLenum s) {
		const char *v = (const char *) glGetString(s);
		LOGI("GL %s = %s\n", name, v);
	}

	extern void checkGlError(const char *op) {
		for (GLint error = glGetError(); error; error = glGetError()) {
			LOGI("after %s() glError (0x%x)\n", op, error);
		}
	}

	GLuint GLES2Lesson::loadShader(GLenum shaderType, const char *pSource) {
		auto shader = glCreateShader(shaderType);
		if (shader) {
			glShaderSource(shader, 1, &pSource, NULL);
			glCompileShader(shader);
			GLint compiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (!compiled) {
				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
				if (infoLen) {
					char *buf = (char *) malloc(infoLen);
					if (buf) {
						glGetShaderInfoLog(shader, infoLen, NULL, buf);
						LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
						free(buf);
					}
					glDeleteShader(shader);
					shader = 0;
				}
			}
		}
		return shader;
	}

	GLuint GLES2Lesson::createProgram(const char *pVertexSource, const char *pFragmentSource) {
		auto vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
		if (!vertexShader) {
			return 0;
		}

		auto pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
		if (!pixelShader) {
			return 0;
		}

		auto program = glCreateProgram();
		if (program) {
			glAttachShader(program, vertexShader);
			checkGlError("glAttachShader");
			glAttachShader(program, pixelShader);
			checkGlError("glAttachShader");
			glLinkProgram(program);
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE) {
				GLint bufLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
				if (bufLength) {
					char *buf = (char *) malloc(bufLength);
					if (buf) {
						glGetProgramInfoLog(program, bufLength, NULL, buf);
						LOGE("Could not link program:\n%s\n", buf);
						free(buf);
					}
				}
				glDeleteProgram(program);
				program = 0;
			}
		}
		return program;
	}

	void GLES2Lesson::printVerboseDriverInformation() {
		printGLString("Version", GL_VERSION);
		printGLString("Vendor", GL_VENDOR);
		printGLString("Renderer", GL_RENDERER);
		printGLString("Extensions", GL_EXTENSIONS);
	}

	GLES2Lesson::GLES2Lesson() {
//start off as identity - late we will init it with proper values.
		cubeTransformMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::mat4(1.0f);
		vertexAttributePosition = 0;
		modelMatrixAttributePosition = 0;
		projectionMatrixAttributePosition = 0;
		gProgram = 0;
	}

	GLES2Lesson::~GLES2Lesson() {
		LOGI("Destroying the renderer");

		if ( kShouldDestroyThingsManually ) {
			for (auto &texture : mTextures) {
				glDeleteTextures(1, &(texture->mTextureId));
			}
			deleteVBOs();
		}

	}

	bool GLES2Lesson::init(float w, float h, const std::string &vertexShader,
	                       const std::string &fragmentShader) {

		printVerboseDriverInformation();

		gProgram = createProgram(vertexShader.c_str(), fragmentShader.c_str());

		if (!gProgram) {
			LOGE("Could not create program.");
			return false;
		}

		fetchShaderLocations();

		glViewport(0, 0, w, h);
		checkGlError("glViewport");

		projectionMatrix = glm::perspective(45.0f, w / h, 0.1f, 100.0f);

		createVBOs();

		for (auto &bitmap : mBitmaps) {
			mTextures.push_back( std::make_shared<Texture>(uploadTextureData(bitmap), bitmap));
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CW);
		glDepthMask(true);
		return true;
	}

	glm::mat4 getCubeTransform(glm::vec3 translation) {
		glm::mat4 identity = glm::mat4(1.0f);
		glm::mat4 translated = glm::translate(identity, translation);

		return translated;
	}

	void GLES2Lesson::resetTransformMatrices() {

		glm::mat4 viewMatrix = glm::lookAt(glm::vec3( 10.0f, 10.0f, 0.0f + ( -20.0f + cameraPosition.y ) / 2.0f ),
		                                   glm::vec3( cameraPosition.x, -1.0f, (-20.0f + cameraPosition.y) - 10.0f ),
		                                   glm::vec3(0.0f, 1.0, 0.0f));

		glUniformMatrix4fv(uView, 1, false, &viewMatrix[0][0]);
	}

	void GLES2Lesson::fetchShaderLocations() {

		vertexAttributePosition = glGetAttribLocation(gProgram, "aPosition");
		modelMatrixAttributePosition = glGetUniformLocation(gProgram, "uModel");
		projectionMatrixAttributePosition = glGetUniformLocation(gProgram, "uProjection");
		samplerUniformPosition = glGetUniformLocation(gProgram, "sTexture");
		textureCoordinatesAttributePosition = glGetAttribLocation(gProgram, "aTexCoord");
		uView = glGetUniformLocation(gProgram, "uView");
	}

	void GLES2Lesson::drawGeometry(const int vertexVbo, const int indexVbo, int vertexCount,
	                               const glm::mat4 &transform) {

		glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
		glEnableVertexAttribArray(vertexAttributePosition);
		glEnableVertexAttribArray(textureCoordinatesAttributePosition);

		//0 is for texturing unit 0 (since we never changed it)
		glUniform1i(samplerUniformPosition, 0);

		glUniformMatrix4fv(modelMatrixAttributePosition, 1, false, &transform[0][0]);
		glVertexAttribPointer(vertexAttributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
		glVertexAttribPointer(textureCoordinatesAttributePosition, 2, GL_FLOAT, GL_TRUE,
		                      sizeof(float) * 5, (void *) (sizeof(float) * 3));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);
		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(vertexAttributePosition);
		glDisableVertexAttribArray(textureCoordinatesAttributePosition);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void GLES2Lesson::deleteVBOs() {
		glDeleteBuffers(1, &vboCubeVertexDataIndex);
		glDeleteBuffers(1, &vboCubeVertexIndicesIndex);

		glDeleteBuffers(1, &vboFloorVertexDataIndex);
		glDeleteBuffers(1, &vboFloorVertexIndicesIndex);

		glDeleteBuffers(1, &vboBillboardVertexDataIndex);
		glDeleteBuffers(1, &vboBillboardVertexIndicesIndex);

	}

	void GLES2Lesson::createVBOs() {
		//walls
		glGenBuffers(1, &vboCubeVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboCubeVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float) * 5, cubeVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboCubeVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboCubeVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), cubeIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//characters
		glGenBuffers(1, &vboBillboardVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboBillboardVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, billboardVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glGenBuffers(1, &vboBillboardVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboBillboardVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), billboardIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//floor
		glGenBuffers(1, &vboFloorVertexDataIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboFloorVertexDataIndex);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 5, floorVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboFloorVertexIndicesIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFloorVertexIndicesIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), floorIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

	void GLES2Lesson::clearBuffers() {
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClearDepthf(1.0f);
		checkGlError("glClearColor");
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		checkGlError("glClear");
	}

	void GLES2Lesson::setPerspective() {
		glUniformMatrix4fv(projectionMatrixAttributePosition, 1, false, &projectionMatrix[0][0]);
	}

	void GLES2Lesson::prepareShaderProgram() {
		glUseProgram(gProgram);
		checkGlError("glUseProgram");
	}

	void GLES2Lesson::render(std::array<std::array<int, 20>, 20> map, std::array<std::array<int, 20>, 20> actors, std::array<std::array<int, 20>, 20> splats) {
		clearBuffers();
		prepareShaderProgram();
		setPerspective();

		resetTransformMatrices();

		for (int z = 0; z < 20; ++z) {
			for (int x = 0; x < 20; ++x) {

				int tile = map[19 - z ][ x ];
				int actor = actors[ 19 - z][ x ];
				int splatFrame = splats[ 19 - z ][ x ];
				bool isCursorPoint = ( ( x == static_cast<int>(this->cursorPosition.x) ) && ( ( 19 - z ) == static_cast<int>(this->cursorPosition.y)) );
				glBindTexture(GL_TEXTURE_2D, mTextures[ ( isCursorPoint ? ETextures::Cursor : ETextures::Grass ) ]->mTextureId );

				drawGeometry(vboFloorVertexDataIndex,
				             vboFloorVertexIndicesIndex,
				             6,
				             getCubeTransform(glm::vec3(-10 + (x * 2), -5.0f, -10 + (-z * 2)))
				);

				if ( ETextures::Bricks <= tile && tile <= ETextures::BricksCandles ) {
					glBindTexture(GL_TEXTURE_2D, mTextures[ tile ]->mTextureId );
					drawGeometry(vboCubeVertexDataIndex,
					             vboCubeVertexIndicesIndex,
					             24,
					             getCubeTransform(glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2)))
					);


					glBindTexture(GL_TEXTURE_2D, mTextures[ ETextures::Top ]->mTextureId );

					drawGeometry(vboFloorVertexDataIndex,
					             vboFloorVertexIndicesIndex,
					             6,
					             getCubeTransform(glm::vec3(-10 + (x * 2), -3.0f, -10 + (-z * 2)))
					);

				}

				if ( actor > ETextures::BricksCandles ) {
					glBindTexture(GL_TEXTURE_2D, mTextures[ actor ]->mTextureId );
					drawGeometry(vboBillboardVertexDataIndex,
					             vboBillboardVertexIndicesIndex,
					             6,
					             getCubeTransform(glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2)))
					);
				}

				if ( splatFrame > -1 ) {
					glBindTexture(GL_TEXTURE_2D, mTextures[ splatFrame + ETextures::Splat0  ]->mTextureId );
					drawGeometry(vboBillboardVertexDataIndex,
							vboBillboardVertexIndicesIndex,
					6,
					getCubeTransform(glm::vec3(-10 + (x * 2), -4.0f, -10 + (-z * 2)))
					);
				}


			}
		}
	}

	void GLES2Lesson::setTexture(std::vector<std::shared_ptr<NativeBitmap>> textures) {
		mBitmaps.clear();
		mBitmaps.insert( mBitmaps.end(), textures.begin(), textures.end());
	}

	void GLES2Lesson::tick() {
	}

	void GLES2Lesson::shutdown() {
		LOGI("Shutdown!\n");
	}

	void GLES2Lesson::setCameraPosition(float x, float y) {
		this->cameraPosition = glm::vec2{ x, y };
	}

	void GLES2Lesson::setCursorAt(float x, float y) {
		this->cursorPosition = glm::vec2{ x, y };
	}
}