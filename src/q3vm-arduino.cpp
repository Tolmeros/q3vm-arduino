#include "q3vm-arduino.h"


/* Callback from the VM: system function call */
/*
intptr_t systemCalls(vm_t* vm, intptr_t* args)
{
    
}
*/

AQ3VM::AQ3VM(const uint32_t data_stack_size, const uint32_t op_stack_size,
		    const vmInterpreterMode_t interpreter_mode) {
	this->data_stack_size = data_stack_size;
	this->op_stack_size = op_stack_size;
	this->interpreter_mode = interpreter_mode;

	systemCalls= &defaultSystemCalls;
	//systemCalls = defaultSystemCalls;

}

void AQ3VM::useImage(uint8_t* image, const uint32_t size) {
	this->image = image;
	this->image_size = size;
	this->use_image = true;
}


int AQ3VM::create() {
	return VM_Create(&vm, module_name, image, image_size, systemCalls);
}

void AQ3VM::setModuleName(char* name){
	strlcpy(module_name, name, sizeof(module_name));
}


intptr_t AQ3VM::call(int command) {
	return VM_Call(&vm, command);
}

void AQ3VM::mem_free() {
	VM_Free(&vm);
}

//intptr_t AQ3VM::defaultSystemCalls(vm_t* vm, intptr_t* args) {
intptr_t defaultSystemCalls(vm_t* vm, intptr_t* args) {
	const int id = -1 - args[0];

    switch (id)
    {
    case -1: /* PRINTF */
        return Serial.printf("Q3VM out: %s", (const char*)VMA(1, vm));

    case -2: /* ERROR */
        return Serial.printf("Q3WM error: %s", (const char*)VMA(1, vm));

    case -3: /* MEMSET */
        if (VM_MemoryRangeValid(args[1] /*addr*/, args[3] /*len*/, vm) == 0)
        {
            memset(VMA(1, vm), args[2], args[3]);
        }
        return args[1];

    case -4: /* MEMCPY */
        if (VM_MemoryRangeValid(args[1] /*addr*/, args[3] /*len*/, vm) == 0 &&
            VM_MemoryRangeValid(args[2] /*addr*/, args[3] /*len*/, vm) == 0)
        {
            memcpy(VMA(1, vm), VMA(2, vm), args[3]);
        }
        return args[1];

    default:
        Serial.printf("Q3WM Bad system call: %ld\n", (long int)args[0]);
    }
    return 0;
}