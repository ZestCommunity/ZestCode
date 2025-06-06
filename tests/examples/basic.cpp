#include <stdint.h>

extern "C" {
void vexDisplayPrintf(int32_t xpos, int32_t ypos, uint32_t bOpaque, const char* format, ...);
}

int main() {
    vexDisplayPrintf(10, 60, 1, "Hello World!\n");
}