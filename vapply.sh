#!/bin/bash
#svn up
make -j3
./src/vislcg3 -C UTF-8 -g ~/parsers/dansk/etc/dancg --grammar-only --grammar-bin dancg.bin3
mkdir -p vapply
cd vapply
rm -fv callgrind.*
rm -fv annotated
g++ -DHAVE_BOOST -I~/tmp/boost_1_44_0 -pipe -Wall -Wextra -Wno-deprecated -O3 -g3 -fno-rtti -ffor-scope -ltcmalloc -licuio -licuuc ../src/all_vislcg3.cpp -o vislcg3.debug
head -n 2000 ../comparison/arboretum_stripped.txt | valgrind --tool=callgrind --compress-strings=no --compress-pos=no --collect-jumps=yes --collect-systime=yes ./vislcg3.debug -v -C UTF-8 -g ../dancg.bin3 > output.txt
#head -n 2000 ../comparison/arboretum_stripped.txt | valgrind ./vislcg3.debug -v -C UTF-8 -g ../dancg.bin3 > output.txt
callgrind_annotate --tree=both --auto=yes > annotated
