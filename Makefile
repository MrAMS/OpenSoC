soc:
	./ysyx_openSoC.py --cpu-type picorv32 --integrated-sram-size 0x4000 --build --doc

demo:
	cd software && litex_bare_metal_demo --build-path ../build/myplatform

serial-boot:
	litex_term /dev/ttyACM0 --kernel ./software/demo.bin

.PHONY: soc demo serial-boot