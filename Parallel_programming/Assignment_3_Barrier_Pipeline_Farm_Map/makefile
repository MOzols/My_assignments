barrier_cpps=$(subst .cpp,,$(wildcard barrier*.cpp))
other_cpps=$(filter-out $(barrier_cpps),$(subst .cpp,,$(wildcard *.cpp)))
odir=./bin
barrier_nprocesses=$(shell echo $$(($$(nproc)*2+11)))

$(barrier_cpps):
	@mkdir -p -m 700 ${odir} && rm -rf ${odir}/*
	mpicxx -O2 -pedantic -Wall $@.cpp -o ${odir}/$@ && mpirun --oversubscribe --quiet -np ${barrier_nprocesses} ${odir}/$@

$(other_cpps):
	@mkdir -p -m 700 ${odir} && rm -rf ${odir}/*
	mpicxx -O2 -pedantic -Wall $@.cpp -o ${odir}/$@ && mpirun --oversubscribe --quiet -np `${odir}/$@ np` ${odir}/$@

clean:
	@rm -rf ./lab3.zip ${odir}

edit:
	@which code >/dev/null && code -n $$(ls -t barrier*pp)  $$(ls -t *.cpp) common.hpp makefile || echo "visual studio code not installed :("

install:
	sudo apt-get install libopenmpi-dev openmpi-bin

zip:
	@rm -f ./lab3.zip && zip ./lab3.zip barrier.cpp barrier.hpp seq.cpp common.hpp makefile
