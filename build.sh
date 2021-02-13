gcc -Wall -O2 -DFUSE_USE_VERSION=31 -Iinc -c flu.c -o flu.o
gcc -Wall -O2 -DFUSE_USE_VERSION=31 -Iinc -c posix_structs.c -o posix_structs.o
gcc -Wall -O2 -DFUSE_USE_VERSION=31 -Iinc -c errno.c -o errno.o
gcc -Wall -O2 -DFUSE_USE_VERSION=31 -Iinc -c compat.c -o compat.o
gcc -shared compat.o errno.o flu.o posix_structs.o -o libflu.so -s lib/libfuse3.so