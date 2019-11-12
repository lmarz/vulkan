#include "triangle.h"

Model loadModel(const char* path) {
    cgltf_options options = {};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if(result != cgltf_result_success) { printf("Error while loading %s\n", path); exit(-1); }
    result = cgltf_load_buffers(&options, data, path);
    if(result != cgltf_result_success) { printf("Error while loading buffers of %s\n", path); exit(-1); }
    
    Model model;
    model.verticesCount = data->meshes[0].primitives[0].attributes[0].data->count;
    model.vertices = (Vertex*)malloc(model.verticesCount * sizeof(Vertex));

    for(int i = 0; i < data->meshes[0].primitives[0].attributes_count; i++) {
        cgltf_attribute attr = data->meshes[0].primitives[0].attributes[i];

        // Position
        if(attr.type == cgltf_attribute_type_position) {
            float* pos = (float*)malloc(attr.data->buffer_view->size);
            memcpy(pos, attr.data->buffer_view->buffer->data + attr.data->buffer_view->offset, attr.data->buffer_view->size);
            int k = 0;
            for(int j = 0; j < model.verticesCount; j++) {
                model.vertices[j].position[0] = pos[k];
                model.vertices[j].position[1] = pos[k+1];
                model.vertices[j].position[2] = pos[k+2];
                k += 3;
            }
            free(pos);

        // Normal
        } else if(attr.type == cgltf_attribute_type_normal) {
            float* normal = (float*)malloc(attr.data->buffer_view->size);
            memcpy(normal, attr.data->buffer_view->buffer->data + attr.data->buffer_view->offset, attr.data->buffer_view->size);
            int k = 0;
            for(int j = 0; j < model.verticesCount; j++) {
                model.vertices[i].normal[0] = normal[k];
                model.vertices[i].normal[1] = normal[k+1];
                model.vertices[i].normal[2] = normal[k+2];
                k += 3;
            }
            free(normal);

        // UV
        } else if(attr.type == cgltf_attribute_type_texcoord) {
            float* uv = (float*)malloc(attr.data->buffer_view->size);
            memcpy(uv, attr.data->buffer_view->buffer->data + attr.data->buffer_view->offset, attr.data->buffer_view->size);
            int k = 0;
            for(int j = 0; j < model.verticesCount; j++) {
                model.vertices[j].uv[0] = uv[k];
                model.vertices[j].uv[1] = uv[k+1];
                k += 2;
            }
            free(uv);
        }

    }

    // Indices
    cgltf_accessor* indi = data->meshes[0].primitives[0].indices;
    uint16_t* indices = (uint16_t*)malloc(indi->count * sizeof(uint16_t));
    memcpy(indices, indi->buffer_view->buffer->data + indi->buffer_view->offset, indi->buffer_view->size);
    model.indicesCount = indi->count;
    model.indices = indices;

    cgltf_free(data);

    return model;
}

void destroyModel(Model model) {
    free(model.vertices);
    free(model.indices);
}