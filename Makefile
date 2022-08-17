run: lib_client main
	./build/main build/lib_client.so

lib_client:
	g++ -shared -fPIC -o build/lib_client.so src/library.cpp

main:
	g++ -o build/main src/main.cpp -ldl -pthread

clean:
	rm -rf build/*