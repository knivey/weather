#!/bin/bash
rm -f ultimateq.grpc.pb.cc  ultimateq.grpc.pb.h  ultimateq.pb.cc  ultimateq.pb.h
rm -f ultimateq.proto
wget https://raw.githubusercontent.com/aarondl/ultimateq/master/api/ultimateq.proto
protoc -I ./ --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ultimateq.proto
protoc -I ./ --cpp_out=. ultimateq.proto
