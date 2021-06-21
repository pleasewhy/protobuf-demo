# make默认执行的target为第一个target
# 可以通过.DEFAULT_GOAL修改
.DEFAULT_GOAL := run

LINK_LIB := `pkg-config --cflags --libs protobuf`

proto_dir = ./proto
proto_cpp_file = $(proto_dir)/game_store.pb.cc
cpp_file = main.cpp


%.pb.cc: %.proto
	protoc --cpp_out=. $^

test: $(proto_cpp_file) $(cpp_file)
	g++ -std=c++11 $(cpp_file) $(proto_cpp_file) -I. -o test $(LINK_LIB)

run: test
	./test

clean:
	rm test

clean-proto:
	rm $(proto_dir)/*.pb.cc \
		 $(proto_dir)/*.pb.h

.PHONY: clean clean-proto