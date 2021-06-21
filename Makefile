# 对于没有依赖的target，需要将其
# 指明为伪目标
.PHONY: clean clean-proto

# make默认执行的target为第一个target
# 可以通过.DEFAULT_GOAL修改
.DEFAULT_GOAL := run

# 获取protobuf静态链接库的位置
LINK_LIB := `pkg-config --cflags --libs protobuf`


proto_dir = ./proto
proto_cpp_file = $(proto_dir)/game_store.pb.cc
cpp_file = main.cpp

# 编译.proto文件
# $^代表冒号右边匹配到的文件,即requirement
# $@代表冒号左边匹配到的文件,即target
%.pb.cc: %.proto
	protoc --cpp_out=. $^ 

# 运行test target 需要game_store.pb.cc和main.cpp
# 如果这两个文件没有被修改过,那么该target的command
# 不会被执行
test: $(proto_cpp_file) $(cpp_file)
	g++ -std=c++11 $(cpp_file) $(proto_cpp_file) -I. -o test $(LINK_LIB)

run: test
	./test

clean:
	rm test

clean-proto:
	rm $(proto_dir)/*.pb.cc \
		 $(proto_dir)/*.pb.h