#!/bin/bash

if [ -z $1 ] 
then
	echo "Must specify input C++ file"
fi

[ $2 ] && png=$2 || png=callgraph.png

clang++ -std=c++11 -S -emit-llvm $1 -o - | opt -analyze -std-link-opts -dot-callgraph
cat callgraph.dot | c++filt -p | sed 's/>/\\>/g; s/-\\>/->/g; s/</\\</g' | awk '/external node/{id=$1} $1 != id' | dot -Tpng -o $png
rm callgraph.dot

