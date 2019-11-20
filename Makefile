INCLUDE=-DDEBUG -I. -I./external/cglm/include -I./external/cgltf -I./external/stb
LIBS=-lglfw -lvulkan -lm

all: vulkan res/shaders/vert.spv res/shaders/frag.spv
	@echo "Starting vulkan ..."
	@./vulkan

vulkan: vulkan.o
	@echo "||| vulkan.o --->>> vulkan |||"
	@gcc -o vulkan vulkan.o $(LIBS)

vulkan.o: vulkan.c
	@echo "||| vulkan.c --->>> vulkan.o |||"
	@gcc -c vulkan.c $(INCLUDE)

res/shaders/vert.spv: res/shaders/shader.vert
	@echo "||| shader.vert --->>> vert.spv |||"
	@glslangValidator --target-env vulkan1.1 -o res/shaders/vert.spv res/shaders/shader.vert

res/shaders/frag.spv: res/shaders/shader.frag
	@echo "||| shader.frag --->>> frag.spv |||"
	@glslangValidator --target-env vulkan1.1 -o res/shaders/frag.spv res/shaders/shader.frag

clean:
	@echo "Cleaning up ..."
	@rm vulkan res/shaders/vert.spv res/shaders/frag.spv