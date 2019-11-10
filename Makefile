INCLUDE=-DDEBUG -I. -I./external/cglm/include
LIBS=-lglfw -lvulkan

all: triangle res/shaders/vert.spv res/shaders/frag.spv
	@echo "Starting triangle ..."
	@./triangle

triangle: triangle.o
	@echo "||| triangle.o --->>> triangle |||"
	@gcc -o triangle triangle.o $(LIBS)

triangle.o: triangle.c
	@echo "||| triangle.c --->>> triangle.o |||"
	@gcc -c triangle.c $(INCLUDE)

res/shaders/vert.spv: res/shaders/shader.vert
	@echo "||| shader.vert --->>> vert.spv |||"
	@glslangValidator --target-env vulkan1.1 -o res/shaders/vert.spv res/shaders/shader.vert

res/shaders/frag.spv: res/shaders/shader.frag
	@echo "||| shader.frag --->>> frag.spv |||"
	@glslangValidator --target-env vulkan1.1 -o res/shaders/frag.spv res/shaders/shader.frag

clean:
	@echo "Cleaning up ..."
	@rm triangle triangle.o res/shaders/vert.spv res/shaders/frag.spv