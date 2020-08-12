
#if defined(ESP8266)
  #include "FS.h"
#elif defined(ESP32)
  #include "SPIFFS.h"
#endif

//#include "heap/include/esp_heap_caps.h"

#define Com_Printf Serial.printf
//#include "q3vm-arduino.h"
//#include "vm.h"
#include "q3vm.h"

#if defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#endif

//#define TEST_ALLOC

//Q3VM test_vm();

#define FILENAME "/bytecode.qvm"

intptr_t systemCalls(vm_t* vm, intptr_t* args);
uint8_t* loadImage(const char* filepath, int* size);

void setup() {
  Serial.begin(115200);
  Serial.println();
  uint32_t free_mem = system_get_free_heap_size();
  Serial.printf("Free heap %d\n", free_mem);

  if (!SPIFFS.begin()) {
    Serial.println(F("SPIFFS not mounted. Try to format"));
    if (!SPIFFS.format()) {
      Serial.println(F("SPIFFS format succesful"));
    }
  } else {
    Serial.println(F("SPIFFS mounted"));
  }

  free_mem = system_get_free_heap_size();
  Serial.printf("Free heap %d\n", free_mem);

  vm_t vm;
  int  retVal = -1;
  int  imageSize;

  uint32_t alltime = millis();
  uint8_t* image    = loadImage(FILENAME, &imageSize);

  #ifdef TEST_ALLOC
  uint8_t* test_ram = (uint8_t*)malloc(10240);
  #endif
  
  Serial.printf("Image size : %d\n", imageSize);

  if (!image) {
    return;
  }

  #ifdef TEST_ALLOC
  if (!test_ram) {
    Serial.println(F("test_ram is not allocated"));
  }
  #endif

  free_mem = system_get_free_heap_size();
  Serial.printf("Free heap %d\n", free_mem);

  if (VM_Create(&vm, FILENAME, image, imageSize, systemCalls) == 0) {
    Serial.println(F("VM is created"));
    
    free_mem = system_get_free_heap_size();
    Serial.printf("Free heap %d\n", free_mem);

    uint32_t runtime = millis();
    
    retVal = VM_Call(&vm, 0);
    Serial.printf("Run time %d ms\n", millis()-runtime);
  }

  VM_Free(&vm);
  free(image);
  
  #ifdef TEST_ALLOC
  free(test_ram);
  #endif
  
  Serial.printf("VM return value is %d\n", retVal);
  Serial.printf("All time %d ms\n", millis()-alltime);
  
  free_mem = system_get_free_heap_size();
  Serial.printf("Free heap %d\n", free_mem);
}

void loop() {
  // put your main code here, to run repeatedly:

}

uint8_t* loadImage(const char* filepath, int* size)
{
    File f;
    uint8_t* image = NULL; /* bytecode buffer */
    int      sz;           /* bytecode file size */

    

    *size = 0;

    //f = SPIFFS.open(filepath, "rb");
    f = SPIFFS.open(filepath, "r");
    
    if (!f)
    {
        Serial.printf("Failed to open file %s.\n", filepath);
        return NULL;
    }
    /* calculate file size */
    sz = f.size();
    Serial.printf("File size is %d.\n", sz);
    if (sz < 1)
    {
        f.close();
        return NULL;
    }

    image = (uint8_t*)malloc(sz);
    if (!image)
    {
        f.close();
        Serial.printf("Failed to read file %s.\n", filepath);
        return NULL;
    }

    
    //if (fread(image, 1, sz, f) != (size_t)sz)
    if (f.read(image, sz) != (size_t)sz)
    {
        free(image);
        f.close();
        return NULL;
    }

    f.close();
    *size = sz;
    return image;
}

void Com_Error(vmErrorCode_t level, const char* error)
{
    Serial.printf("Q3VM Err (%i): %s\n", level, error);

    uint32_t free_mem = system_get_free_heap_size();
    Serial.printf("Q3VM Err Free heap %d\n", free_mem);

    //exit(level);
}

/* Callback from the VM for memory allocation */
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type)
{
    //(void)vm;
    //(void)type;
    Serial.print(F("VM malloc size:"));
    Serial.println(size);
    return malloc(size);
    //return heap_caps_malloc(size, MALLOC_CAP_32BIT);
}

/* Callback from the VM for memory release */
void Com_free(void* p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    free(p);
}

/* Callback from the VM: system function call */
intptr_t systemCalls(vm_t* vm, intptr_t* args)
{
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

/*
void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm) {
  
}
*/
