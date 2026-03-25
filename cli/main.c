#include <stdio.h>
#include <string.h>
#include "nia/common.h"
#include "nia_min/service.h"
#include "nia_fw/service.h"
#include "nia_platform/platform.h"
#include "simulator.h"

static nia_status_t tool_cursor_move(const nia_tool_call_t *call, nia_tool_result_t *result)
{
    (void)call;
    const char *msg = "cursor moved";
    size_t len = strlen(msg);
    memcpy(result->data, msg, len);
    result->len = len;
    result->status = NIA_OK;
    return NIA_OK;
}

static nia_status_t tool_select(const nia_tool_call_t *call, nia_tool_result_t *result)
{
    (void)call;
    const char *msg = "selection made";
    size_t len = strlen(msg);
    memcpy(result->data, msg, len);
    result->len = len;
    result->status = NIA_OK;
    return NIA_OK;
}

static void print_usage(void)
{
    printf("NIA — Neural Interface Adapter v%s\n\n", NIA_VERSION_STRING);
    printf("Usage: nia <command> [options]\n\n");
    printf("Commands:\n");
    printf("  run-min       Run NIA-Min with simulator provider\n");
    printf("  run-fw        Run NIA-Framework with simulator provider\n");
    printf("  info          Show platform information\n");
    printf("  version       Show version\n");
    printf("  help          Show this help\n");
}

static int cmd_info(void)
{
    nia_platform_init();
    nia_platform_info_t pi = nia_platform_info();

    printf("NIA v%s\n", NIA_VERSION_STRING);
    printf("Platform: %s (%s)\n", pi.os_name, pi.arch);
    printf("Real-time: %s\n", pi.realtime_capable ? "yes" : "no");
    printf("Hardware:  %s\n", pi.hardware_access  ? "yes" : "no");
    return 0;
}

static int cmd_run_min(int ticks)
{
    nia_config_t cfg;
    nia_config_load_defaults(&cfg, NIA_VARIANT_MIN);

    nia_min_service_t svc;
    nia_status_t st = nia_min_service_init(&svc, &cfg, &nia_provider_simulator_ops);
    if (st != NIA_OK) {
        fprintf(stderr, "init failed: %s\n", nia_status_str(st));
        return 1;
    }

    /* Register mappings */
    nia_min_mapper_add(&svc.mapper, "move_left",    "ui.cursor.move");
    nia_min_mapper_add(&svc.mapper, "move_right",   "ui.cursor.move");
    nia_min_mapper_add(&svc.mapper, "select",       "ui.select");
    nia_min_mapper_add(&svc.mapper, "scroll_up",    "ui.cursor.move");
    nia_min_mapper_add(&svc.mapper, "scroll_down",  "ui.cursor.move");
    nia_min_mapper_add(&svc.mapper, "activate",     "ui.select");
    nia_min_mapper_add(&svc.mapper, "deactivate",   "ui.select");

    /* Register tools */
    nia_tool_entry_t cursor_tool = { .name = "ui.cursor.move", .description = "Move cursor", .exec = tool_cursor_move };
    nia_tool_entry_t select_tool = { .name = "ui.select", .description = "Select item", .exec = tool_select };
    nia_min_tool_bridge_register(&svc.tool_bridge, &cursor_tool);
    nia_min_tool_bridge_register(&svc.tool_bridge, &select_tool);

    st = nia_min_service_start(&svc);
    if (st != NIA_OK) {
        fprintf(stderr, "start failed: %s\n", nia_status_str(st));
        return 1;
    }

    printf("NIA-Min running (simulator, %d ticks)...\n", ticks);
    for (int i = 0; i < ticks; i++) {
        nia_min_service_tick(&svc);
    }

    nia_min_service_stats(&svc);
    nia_min_service_shutdown(&svc);
    return 0;
}

static int cmd_run_fw(int ticks)
{
    nia_config_t cfg;
    nia_config_load_defaults(&cfg, NIA_VARIANT_FRAMEWORK);

    nia_fw_service_t svc;
    nia_status_t st = nia_fw_service_init(&svc, &cfg);
    if (st != NIA_OK) {
        fprintf(stderr, "init failed: %s\n", nia_status_str(st));
        return 1;
    }

    st = nia_fw_service_add_provider(&svc, &nia_provider_simulator_ops, "simulator");
    if (st != NIA_OK) {
        fprintf(stderr, "add provider failed: %s\n", nia_status_str(st));
        return 1;
    }

    st = nia_fw_service_start(&svc);
    if (st != NIA_OK) {
        fprintf(stderr, "start failed: %s\n", nia_status_str(st));
        return 1;
    }

    printf("NIA-Framework running (simulator, %d ticks)...\n", ticks);
    for (int i = 0; i < ticks; i++) {
        nia_fw_service_tick(&svc);
    }

    nia_fw_service_stats(&svc);
    nia_fw_service_shutdown(&svc);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage();
        return 0;
    }

    nia_log_set_level(NIA_LOG_INFO);

    const char *cmd = argv[1];

    if (strcmp(cmd, "version") == 0) {
        printf("NIA v%s\n", NIA_VERSION_STRING);
        return 0;
    }

    if (strcmp(cmd, "help") == 0) {
        print_usage();
        return 0;
    }

    if (strcmp(cmd, "info") == 0) {
        return cmd_info();
    }

    int ticks = 30;
    if (argc >= 3) {
        ticks = atoi(argv[2]);
        if (ticks <= 0) ticks = 30;
    }

    if (strcmp(cmd, "run-min") == 0) {
        return cmd_run_min(ticks);
    }

    if (strcmp(cmd, "run-fw") == 0) {
        return cmd_run_fw(ticks);
    }

    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage();
    return 1;
}
