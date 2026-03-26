#include <WidgetEx/LayoutWidget.h>
#include <WidgetEx/LayoutWin.h>
#include <Widget/WidgetX.h>

#include <x++/X.h>
#include <unistd.h>
#include <stdlib.h>

#include <tinyjson/tinyjson.h>

#include "js.h"
#include "mbc.h"
#include "native_UniObject.h"

/**
load extra native libs.
*/

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

void reg_native_wjs(vm_t* vm, void* arg);
bool js_compile(bytecode_t *bc, const char* input);

static inline var_t* vm_load_var(vm_t* vm, const char* name, bool create) {
	node_t* n = vm_load_node(vm, name, create);
	if(n != NULL)
		return n->var;
	return NULL;
}

void reg_all_natives(vm_t* vm);
void reg_mariox_natives(vm_t* vm);
vm_t* init_js(void) {
	vm_t* vm = vm_new(js_compile, 1024, 0);
	vm_init(vm, reg_all_natives, NULL);
	reg_mariox_natives(vm);
	return vm;
}

bool load_wjs(vm_t* vm, const char* fname) {
	if(fname == NULL || fname[0] == 0)
		return false;
	bool res = false;
	if(strstr(fname, ".js") != NULL)
		res = load_js(vm, fname);
	else if(strstr(fname, ".mbc") != NULL) {
		bc_release(&vm->bc);
		res = vm_load_mbc(vm, fname);
	}
	return res;
}

void quit_js(vm_t* vm) {
	vm_close(vm);
}

#ifdef __cplusplus /* __cplusplus */
}
#endif

vm_t* _vm = NULL;
static bool _vm_ready = false;

using namespace Ewok;
#define CLS_Widget "Widget"

static void onMenuItemFunc(MenuItem* it, void* data) {
	(void)it;
	(void)data;
	// 完全忽略事件，直到 VM 准备就绪
	if(!_vm_ready || _vm == NULL) {
		return;
	}
	
	vm_t* vm = _vm;
	klog("onMenuItemFunc: %d\n", it->id);
	var_t* ret = call_m_func_by_name(_vm, NULL, "_onMenuItemEvent", 1, var_new_int(vm, it->id));
	if(ret != NULL) {
		var_unref(ret);
	}
}

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif

static void onEventFunc(Widget* wd, xevent_t* xev, void* arg) {
	klog("onEventFunc: 0x%x, 0x%x\n", wd, arg);
	(void)arg;
	// 完全忽略事件，直到 VM 准备就绪
	if(!_vm_ready || _vm == NULL) {
		return;
	}

	// 验证事件指针有效性
	if(xev == NULL) {
		return;
	}

	// 验证事件类型有效性，防止非法事件类型导致崩溃
	if(xev->type >= XEVT_WIN) {
		return;
	}

	// 验证 Widget 指针有效性，防止使用无效的 Widget 指针导致崩溃
	if(wd == NULL) {
		return;
	}

	vm_t* vm = _vm;

	var_t* evt_arg = var_new_obj(vm, NULL, NULL, NULL);
	var_add(evt_arg, "type", var_new_int(vm, xev->type));
	var_add(evt_arg, "state", var_new_int(vm, xev->state));

	var_t* mouse_arg = var_new_obj(vm, NULL, NULL, NULL);
	var_add(mouse_arg, "x", var_new_int(vm, xev->value.mouse.x));
	var_add(mouse_arg, "y", var_new_int(vm, xev->value.mouse.y));
	var_add(mouse_arg, "rx", var_new_int(vm, xev->value.mouse.rx));
	var_add(mouse_arg, "ry", var_new_int(vm, xev->value.mouse.ry));
	var_add(evt_arg, "mouse", mouse_arg);

	var_t* im_arg = var_new_obj(vm, NULL, NULL, NULL);
	var_add(im_arg, "value", var_new_int(vm, xev->value.im.value));
	var_add(evt_arg, "im", im_arg);

	var_t* var_wd = new_obj(vm, CLS_Widget, 0);
	var_wd->value = wd;
	var_wd->free_func = free_none;

	var_t* ret = call_m_func_by_name(_vm, NULL, "_onWidgetEvent", 2, var_wd, evt_arg);
	if(ret != NULL) {
		var_unref(ret);
	}
}

#ifdef __cplusplus /* __cplusplus */
}
#endif

static int doargs(int argc, char* argv[]) {
	int c = 0;
	while (c != -1) {
		c = getopt (argc, argv, "d");
		if(c == -1)
			break;

		switch (c) {
		case 'd':
			// debug mode
			break;
		case '?':
			return -1;
		default:
			c = -1;
			break;
		}
	}
	return optind;
}

static bool loadWJS(const string& wjs_fname, string& layout_fname, string& js_fname) {
    json_var_t* conf_var = json_parse_file(wjs_fname.c_str());
    if(conf_var == NULL)
        return false;
	
	layout_fname = json_get_str(conf_var, "layout");
	js_fname = json_get_str(conf_var, "js");
    
    json_var_unref(conf_var);
    return true;
}

static void out(const char* str) {
    slog("%s", str);
}

void platform_init(void) {
	_platform_malloc = (void* (*)(uint32_t))malloc;
	_platform_free = free;
    _platform_out = out;
}

int main(int argc, char** argv) {
	platform_init();

	int argind = doargs(argc, argv);
	if(argind < 0) {
		return -1;
	}

	const char* wsj_fname = "";
	if(argind < argc) {
		wsj_fname = argv[argind];
		argind++;
	}

	if(wsj_fname[0] == 0) {
		slog("Usage: %s <xxxx.wjs>\n", argv[0]);
		return -1;
	}

	string layout_fname, js_fname;
	if(!loadWJS(wsj_fname, layout_fname, js_fname)) {
		return -1;
	}

	X x;
	LayoutWin win;
	LayoutWidget* layout = win.getLayoutWidget();
	klog("1 layout: 0x%x\n", layout);

	// 步骤 1：初始化 VM
	_vm = init_js();
	if(_vm == NULL) {
		slog("Failed to initialize VM\n");
		return -1;
	}

	// 步骤 2：注册 natives 和加载 JS
	reg_native_wjs(_vm, layout);
	if(!load_wjs(_vm, js_fname.c_str())) {
		slog("Failed to load JS file: %s\n", js_fname.c_str());
		return -1;
	}
	vm_run(_vm);

	// 步骤 3：标记 VM 准备就绪
	_vm_ready = true;

	// 步骤 4：现在才设置事件回调
	klog("2 layout: 0x%x\n", layout);
	layout->setMenuItemFunc(onMenuItemFunc);
	layout->setEventFunc(onEventFunc);
	klog("3 layout: 0x%x\n", layout);

	// 步骤 5：加载配置和打开窗口
	win.loadConfig(layout_fname.c_str()); // 加载布局文件
	win.open(&x, -1, -1, -1, 0, 0, argv[1], XWIN_STYLE_NORMAL);
	win.setTimer(16);

	widgetXRun(&x, &win);	
	quit_js(_vm);
	return 0;
}
