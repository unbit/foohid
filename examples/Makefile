all: keyboard mouse u2f

binDir:
	mkdir -p bin

keyboard: binDir keyboard.c
	gcc keyboard.c -o bin/virtual_keyboard -framework IOKit

mouse: binDir mouse.c
	gcc mouse.c -o bin/virtual_mouse -framework IOKit

u2f: binDir u2f.c
	gcc u2f.c -o bin/virtual_u2f -framework IOKit
