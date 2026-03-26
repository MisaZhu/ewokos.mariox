#include "mario.h"
#include "native_graph.h"
#include "native_x.h"

void reg_mariox_natives(vm_t* vm) {
	reg_native_graph(vm);
	reg_native_x(vm);
}