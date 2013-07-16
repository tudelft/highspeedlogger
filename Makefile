

all:
	make -C tools
	make -C sw/logger

clean: 
	rm -rf ./hw/*.PcbLib ./hw/*.Harness ./hw/*.SchLib ./hw/*.s4m
	rm -rf ./tools/build
	make mrproper -C ./sw/logger
