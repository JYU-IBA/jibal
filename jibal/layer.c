#include <stdlib.h>
#include <jibal_layer.h>


jibal_layer *jibal_layer_new(jibal_material *material, double thickness) {
    if(!material)
        return NULL;
    jibal_layer *layer=malloc(sizeof(jibal_layer));
    layer->material=material;
    layer->thickness=thickness;
    layer->roughness=0.0;
    return layer;
}

void jibal_layer_free(jibal_layer *layer) {
    if(!layer) {
        return;
    }
    jibal_material_free(layer->material);
    free(layer);
}