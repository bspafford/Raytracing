#include "SSBO.h"

SSBO::SSBO(const void* data, GLuint size) {
	// create
	glGenBuffers(1, &ID);
	// bind
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	// allocate and upload data
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
	// bind ssbo to binding point
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindSlot, ID);
	// unbind
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	bindSlot++;
}