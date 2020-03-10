#include <stdlib.h>
#include <jibal_layer.h>


jibal_layer *jibal_layer_new(jibal_material *material, double thickness) {
    jibal_layer *layer=malloc(sizeof(jibal_layer));
    layer->thickness=thickness;
    layer->roughness=0.0;
    return layer;
}