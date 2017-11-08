

all:
	make -C tools
	make -C sw/logger

clean: 
	rm -rf ./hw/*.PcbLib ./hw/*.Harness ./hw/*.SchLib ./hw/*.s4m
	rm -rf ./tools/build
	make mrproper -C ./sw/logger


html:
	sphinx-build -M html . _build

pdf:
	sphinx-build -M latexpdf . _build


help:
	sphinx-build -M help . _build
