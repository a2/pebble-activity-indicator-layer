# pebble-activity-indicator-layer

A "material design"-style activity indicator for Pebble.

## Installation

```
pebble package install pebble-activity-indicator-layer
```

You must be using a new-style project; install the latest pebble tool and SDK
and run `pebble convert-project` on your app if you aren't.

## Usage

Here is a basic example of how to use the ActivityIndicatorLayer in your app.
For a more detailed example, check out the [demo app](examples/demo/src/demo.c).

```c
static ActivityIndicatorLayer *s_activity_indicator_layer;

...

s_activity_indicator_layer = activity_indicator_layer_create(frame);
activity_indicator_layer_set_animating(s_activity_indicator_layer, true);
layer_add_child(window_layer, (Layer *)s_activity_indicator_layer);
```
