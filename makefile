prebuild:
	@echo Generating f_specs
	./c-atom/tools/fspecgen.py --code --cmake --registry_c ./f_specs_reg.c --f_specs_dirs cube:f_specs catom:c-atom/f_specs/
	@echo Inlining XML configs
	./c-atom/tools/xml2c_inliner.py --cfg_path config/cube/ --out xml_inline_cfgs.c

build:
	@echo Building
	bash -c "rm -r -f build"
	bash -c "mkdir build"
	bash -c "cd build && cmake .. -DTYPE=Cube -DCMAKE_BUILD_TYPE=Release && make ctlst-fmuv5.elf"

flash:
	@echo Firmware downloading
	openocd -f interface/stlink.cfg -f ./bsp/cube/mcu/stm32h753/core/stm32h7.cfg -c "init" -c "program ./build/firmware/ctlst-fmuv5.elf verify reset exit"
