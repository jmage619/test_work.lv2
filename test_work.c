#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>

#define TEST_WORK_URI "http://lv2plug.in/plugins/test_work"

typedef struct {
  int first_run;
  LV2_Worker_Schedule* schedule;
} test_plugin;


static int work_id = 0;

static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double sample_rate, const char* bundle_path,
    const LV2_Feature* const* features) {
  srand(time(NULL));
  test_plugin* plugin = malloc(sizeof(test_plugin));
  
  plugin->schedule = NULL;

  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
      plugin->schedule = (LV2_Worker_Schedule*) features[i]->data;
    }
  }
  if (plugin->schedule == NULL) {
    fprintf(stderr, "Host does not support work:schedule.\n");
    free(plugin);
    return NULL;
  }

  return plugin;
}

static void activate(LV2_Handle instance) {
  test_plugin* plugin = (test_plugin*) instance;
  plugin->first_run = 1;
  fprintf(stderr, "plugin activated!\n");
}

static void deactivate(LV2_Handle instance) {
  fprintf(stderr, "plugin deactivated!\n");
}

static void cleanup(LV2_Handle instance) {
  test_plugin* plugin = (test_plugin*) instance;
  free(plugin);
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
}
static LV2_Worker_Status work(LV2_Handle instance, LV2_Worker_Respond_Function respond,
    LV2_Worker_Respond_Handle handle, uint32_t size, const void* data) {

  int* i = (int*) data;
  //int t = rand() % 3;
  int t = 0;
  fprintf(stderr, "worker %i called! sleeping %i seconds..\n", *i, t);
  sleep(t);
  respond(handle, sizeof(int), i); 
  return LV2_WORKER_SUCCESS;
}
static LV2_Worker_Status work_response(LV2_Handle instance, uint32_t size, const void* data) {
  int* i = (int*) data;
  fprintf(stderr, "response %i called!\n", *i);
  return LV2_WORKER_SUCCESS;
}

static void run(LV2_Handle instance, uint32_t n_samples) {
  test_plugin* plugin = (test_plugin*) instance;
  if (plugin->first_run) {
    int i;
    for (i = 0; i < 10; ++i) {
      plugin->schedule->schedule_work(plugin->schedule->handle, sizeof(int), &work_id);
      fprintf(stderr, "worker %i scheduled!\n", work_id);
      ++work_id;
    }
    plugin->first_run = 0;
  }
}

static const void* extension_data(const char* uri) {
  static const LV2_Worker_Interface worker = { work, work_response, NULL };
  if (!strcmp(uri, LV2_WORKER__interface))
    return &worker;

  return NULL;
}
static const LV2_Descriptor descriptor = {
  TEST_WORK_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  switch (index) {
    case 0:
      return &descriptor;
    default:
      return NULL;
  }
}
