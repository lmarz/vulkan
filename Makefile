# the name of the application
TARGET = vulkan

# the c compiler
CC = gcc

# the object files, which will be created
OBJECTS := vulkan.o
OBJECTS += modelloader.o
OBJECTS += entity.o
OBJECTS += vk/renderer.o
OBJECTS += vk/device.o
OBJECTS += vk/swapchain.o
OBJECTS += vk/pipeline.o
OBJECTS += vk/command.o
OBJECTS += vk/buffer.o
OBJECTS += vk/texture.o

# the DEBUG macro. remove for release
CPPFLAGS = -D DEBUG -g

# the include paths
CFLAGS = -I. -I./external/cglm/include -I./external/cgltf -I./external/stb

# the libraries
LDLIBS = -lglfw -lvulkan -lm

all: $(TARGET) res/shaders/vert.spv res/shaders/frag.spv
	@echo "Starting vulkan ..."
	@./vulkan

$(TARGET): $(OBJECTS)

$(OBJECTS): %.o: %.c

res/shaders/vert.spv: res/shaders/shader.vert
	glslangValidator --target-env vulkan1.2 -o res/shaders/vert.spv res/shaders/shader.vert

res/shaders/frag.spv: res/shaders/shader.frag
	glslangValidator --target-env vulkan1.2 -o res/shaders/frag.spv res/shaders/shader.frag

clean:
	-rm $(TARGET) $(OBJECTS) res/shaders/vert.spv res/shaders/frag.spv