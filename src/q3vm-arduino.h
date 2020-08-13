#pragma once

#include "Arduino.h"
#include "q3vm.h"

typedef enum {
	ALL_IN_RAM,
	ONLY_BYTECODE_IN_RAM,
	ONLY_BYTECODE_FROM_DISK,
} vmInterpreterMode_t;

#define DEFAULT_DATA_STACK_SIZE 	4*1024
#define DEFAULT_OP_STACK_SIZE		1*1024
#define DEFAULT_INTERPRETER_MODE	ALL_IN_RAM

class AQ3VM {
public:
	AQ3VM(const uint32_t data_stack_size=DEFAULT_DATA_STACK_SIZE,
		  const uint32_t op_stack_size=DEFAULT_OP_STACK_SIZE,
		  const vmInterpreterMode_t interpreter_mode=DEFAULT_INTERPRETER_MODE);
	void useImage(uint8_t* image, const uint32_t size);
	int create();
	void setModuleName(char* name);

	intptr_t call(int command);
	void mem_free();

private:
	uint32_t data_stack_size;
	uint32_t op_stack_size;
	vmInterpreterMode_t interpreter_mode;

	char module_name[255];

	uint8_t* image;
	uint32_t image_size;
	bool use_image = false;

	vm_t vm;

	intptr_t (*systemCalls)(vm_t*, intptr_t*);


	//intptr_t defaultSystemCalls(vm_t* vm, intptr_t* args);
};

intptr_t defaultSystemCalls(vm_t* vm, intptr_t* args);