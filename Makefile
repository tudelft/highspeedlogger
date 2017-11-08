

all:
	make -C tools
	make -C sw/logger

clean: 
	rm -rf ./hw/*.PcbLib ./hw/*.Harness ./hw/*.SchLib ./hw/*.s4m
	rm -rf ./tools/build
	rm -rf ./_docs
	make mrproper -C ./sw/logger


html:
	sphinx-build -M html . _docs

pdf:
	sphinx-build -M latexpdf . _docs


linkcheck:
	sphinx-build -M linkcheck . _docs

help:
	sphinx-build -M help . _docs
