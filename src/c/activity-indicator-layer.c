#include "activity-indicator-layer.h"

typedef struct ActivityIndicatorData {
  Animation *animation;
  GColor color;
  uint8_t thickness;
  uint32_t stroke_start;
  uint32_t stroke_end;
  uint32_t rotation;
} ActivityIndicatorData;

#define activity_indicator_layer_animation_implementation(name, type, accessor_type)        \
  static type activity_indicator_layer_get_##name(const Layer *layer) {                     \
    ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);           \
    return data->name;                                                                      \
  }                                                                                         \
                                                                                            \
  static void activity_indicator_layer_set_##name(Layer *layer, type name) {                \
    ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);           \
    data->name = name;                                                                      \
    layer_mark_dirty(layer);                                                                \
  }                                                                                         \
                                                                                            \
  static const PropertyAnimationImplementation property_animation_implementation_##name = { \
    .base = {                                                                               \
      .update = (AnimationUpdateImplementation)property_animation_update_##accessor_type,   \
    },                                                                                      \
    .accessors = {                                                                          \
      .getter.accessor_type = (void *)activity_indicator_layer_get_##name,                  \
      .setter.accessor_type = (void *)activity_indicator_layer_set_##name,                  \
    },                                                                                      \
  };

activity_indicator_layer_animation_implementation(stroke_start, uint32_t, uint32)
activity_indicator_layer_animation_implementation(stroke_end, uint32_t, uint32)
activity_indicator_layer_animation_implementation(rotation, uint32_t, uint32)

static int32_t stroke_start_custom_curve(int32_t progress) {
  if (progress > ANIMATION_NORMALIZED_MAX / 2) {
    return ANIMATION_NORMALIZED_MAX * 4 / 5 + (progress - ANIMATION_NORMALIZED_MAX / 2) * 2 / 5;
  } else {
    return (2 * progress) * 4 / 5;
  }
}

static int32_t stroke_end_custom_curve(int32_t progress) {
  if (progress > ANIMATION_NORMALIZED_MAX * 3 / 5) {
    return ANIMATION_NORMALIZED_MAX;
  } else {
    return progress * 5 / 3;
  }
}

static Animation *activity_indicator_layer_create_animation(Layer *layer) {
  Animation *animations[3];
  const uint32_t duration = 1000;
  uint32_t from = 0, to = TRIG_MAX_ANGLE;

  animations[0] = ({
    PropertyAnimation *property_animation = property_animation_create(&property_animation_implementation_stroke_start, layer, NULL, NULL);
    property_animation_from(property_animation, &from, sizeof(from), true);
    property_animation_to(property_animation, &to, sizeof(to), true);

    Animation *animation = (Animation *)property_animation;
    animation_set_custom_curve(animation, stroke_start_custom_curve);
    animation_set_delay(animation, duration * 2 / 3);
    animation_set_duration(animation, duration);
    animation_set_play_count(animation, ANIMATION_PLAY_COUNT_INFINITE);

    animation;
  });

  animations[1] = ({
    PropertyAnimation *property_animation = property_animation_create(&property_animation_implementation_stroke_end, layer, NULL, NULL);
    property_animation_from(property_animation, &from, sizeof(from), true);
    property_animation_to(property_animation, &to, sizeof(to), true);

    Animation *animation = (Animation *)property_animation;
    animation_set_custom_curve(animation, stroke_end_custom_curve);
    animation_set_duration(animation, duration * 5 / 3);
    animation_set_play_count(animation, ANIMATION_PLAY_COUNT_INFINITE);

    animation;
  });

  animations[2] = ({
    PropertyAnimation *property_animation = property_animation_create(&property_animation_implementation_rotation, layer, NULL, NULL);
    property_animation_from(property_animation, &from, sizeof(from), true);
    property_animation_to(property_animation, &to, sizeof(to), true);

    Animation *animation = (Animation *)property_animation;
    animation_set_curve(animation, AnimationCurveLinear);
    animation_set_duration(animation, duration * 3 / 2);
    animation_set_play_count(animation, ANIMATION_PLAY_COUNT_INFINITE);

    animation;
  });

  return animation_spawn_create_from_array(animations, ARRAY_LENGTH(animations));
}

static void activity_indicator_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);
  if (data->animation == NULL) {
    return;
  }

  int32_t start = data->rotation + data->stroke_start;
  int32_t end = data->rotation + data->stroke_end;

  while (start > end) {
    end += TRIG_MAX_ANGLE;
  }

  if (end - start < DEG_TO_TRIGANGLE(1)) {
    end = start + DEG_TO_TRIGANGLE(1);
  }

  uint8_t thickness = data->thickness;
  graphics_context_set_stroke_color(ctx, data->color);
  graphics_context_set_stroke_width(ctx, thickness);
  graphics_draw_arc(ctx, grect_crop(bounds, (thickness + 1) / 2), GOvalScaleModeFitCircle, start, end);
}

ActivityIndicatorLayer *activity_indicator_layer_create(GRect frame) {
  Layer *layer = layer_create_with_data(frame, sizeof(ActivityIndicatorData));
  layer_set_update_proc(layer, activity_indicator_layer_update_proc);

  ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);
  data->color = GColorBlack;
  data->thickness = 3;

  return (ActivityIndicatorLayer *)layer;
}

void activity_indicator_layer_destroy(ActivityIndicatorLayer *activity_indicator_layer) {
  Layer *layer = (Layer *)activity_indicator_layer;
  ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);
  animation_destroy(data->animation);
  layer_destroy(layer);
}

Layer *activity_indicator_layer_get_layer(const ActivityIndicatorLayer *activity_indicator_layer) {
  return (Layer *)activity_indicator_layer;
}

bool activity_indicator_layer_get_animating(const ActivityIndicatorLayer *activity_indicator_layer) {
  Layer *layer = (Layer *)activity_indicator_layer;
  ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);
  return data->animation != NULL;
}

void activity_indicator_layer_set_animating(ActivityIndicatorLayer *activity_indicator_layer, bool animating) {
  Layer *layer = (Layer *)activity_indicator_layer;
  ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);

  bool is_animating = data->animation != NULL;
  if (is_animating == animating) {
    return;
  }

  Animation *animation = data->animation;
  if (animating) {
    data->animation = animation = activity_indicator_layer_create_animation(layer);
    animation_schedule(animation);
  } else {
    animation_destroy(animation);
    data->animation = NULL;

    layer_mark_dirty(layer);
  }
}

#define activity_indicator_layer_accessors(name, type)                                                    \
  type activity_indicator_layer_get_##name(const ActivityIndicatorLayer *activity_indicator_layer) {      \
    Layer *layer = (Layer *)activity_indicator_layer;                                                     \
    ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);                         \
    return data->name;                                                                                    \
  }                                                                                                       \
                                                                                                          \
  void activity_indicator_layer_set_##name(ActivityIndicatorLayer *activity_indicator_layer, type name) { \
    Layer *layer = (Layer *)activity_indicator_layer;                                                     \
    ActivityIndicatorData *data = (ActivityIndicatorData *)layer_get_data(layer);                         \
    data->name = name;                                                                                    \
    layer_mark_dirty(layer);                                                                              \
  }

activity_indicator_layer_accessors(color, GColor)
activity_indicator_layer_accessors(thickness, uint8_t)
