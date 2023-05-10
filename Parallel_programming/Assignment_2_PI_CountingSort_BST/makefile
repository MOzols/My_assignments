cpps=$(subst .cpp,,$(wildcard *.cpp))
odir=./bin
n=100000000

$(cpps):
	@mkdir -p -m 700 ${odir} && rm -rf ${odir}/$@
	g++ -O1 -Wall -Dnproc=`nproc` -Dcachelinesize=`getconf LEVEL1_DCACHE_LINESIZE` -pedantic -fopenmp -Wall -Wextra -Werror $@.cpp -o ${odir}/$@ && ${odir}/$@ ${n}

edit:
	@which code >/dev/null && code -n *.cpp makefile || echo "visual studio code not installed :("

clean:
	@rm -rf ./bin ./lab2.zip

zip:
	@rm -f ./lab2.zip && zip ./lab2.zip balancetree.cpp countingsort.cpp riemannsumpi.cpp makefile
