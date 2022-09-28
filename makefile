prebuild:
	@echo Generating f_specs
	./c-atom/tools/fspecgen.py --code --cmake --registry_c ./f_specs_reg.c --f_specs_dirs cube:f_specs catom:c-atom/f_specs/
	@echo Inlining XML configs
	./c-atom/tools/xml2c_inliner.py --cfg_path config/cube/ --out xml_inline_cfgs.c
