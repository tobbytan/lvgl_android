package ui

/*
#cgo CFLAGS: -I../third/lvgl/
#cgo LDFLAGS: -L../third/lvgl/build/lib
#cgo LDFLAGS: -llvgl -lEGL -lGLESv1_CM -lGLESv3
#include "lvgl_ui.h"
*/
import "C"

func InitUI() {
	go C.start_lvgl_ui()
}
