all: build 

# install

build:
	make -C shell-pi-side
	make -C shell-unix-side

clean:
	rm -f *~ *.bak
	make -C shell-pi-side clean
	make -C shell-unix-side clean
