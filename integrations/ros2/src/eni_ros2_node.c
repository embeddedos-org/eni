// SPDX-License-Identifier: MIT
// eNI ROS 2 Node — Neural intent publisher and feedback subscriber

#include <stdio.h>
#include <string.h>

#ifdef ENI_HAS_ROS2
#include <rclc/rclc.h>
#include <rclc/executor.h>
#endif

typedef struct {
    const char *node_name;
    const char *intent_topic;
    const char *signal_topic;
    const char *feedback_topic;
    int         publish_rate_hz;
} eni_ros2_config_t;

typedef struct {
    eni_ros2_config_t config;
    int               initialized;
    int               running;
    int               publish_count;
} eni_ros2_node_t;

int eni_ros2_init(eni_ros2_node_t *node, const eni_ros2_config_t *config)
{
    if (!node || !config) return -1;
    memset(node, 0, sizeof(*node));
    node->config = *config;

    if (!node->config.node_name) node->config.node_name = "eni_bci_node";
    if (!node->config.intent_topic) node->config.intent_topic = "/eni/intent";
    if (!node->config.signal_topic) node->config.signal_topic = "/eni/signal";
    if (!node->config.feedback_topic) node->config.feedback_topic = "/eni/feedback";
    if (node->config.publish_rate_hz <= 0) node->config.publish_rate_hz = 30;

#ifdef ENI_HAS_ROS2
    /* TODO: Initialize ROS 2 node, publishers, subscribers
     * rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
     * rcl_init_options_init(&init_options, rcl_get_default_allocator());
     * rclc_support_t support;
     * rclc_support_init(&support, 0, NULL, &init_options);
     * rclc_node_init_default(&node->rcl_node, config->node_name, "", &support);
     */
#endif

    node->initialized = 1;
    printf("[ROS2] Node '%s' initialized\n", node->config.node_name);
    return 0;
}

int eni_ros2_publish_intent(eni_ros2_node_t *node, const char *intent, float confidence, int class_id)
{
    if (!node || !node->initialized || !intent) return -1;

#ifdef ENI_HAS_ROS2
    /* TODO: Publish NeuralIntent message to intent_topic */
#endif

    node->publish_count++;
    printf("[ROS2] Published intent: %s (%.2f) #%d\n", intent, confidence, node->publish_count);
    return 0;
}

int eni_ros2_publish_signal(eni_ros2_node_t *node, const float *samples, int num_channels, uint64_t timestamp)
{
    if (!node || !node->initialized || !samples) return -1;
    (void)num_channels;
    (void)timestamp;

#ifdef ENI_HAS_ROS2
    /* TODO: Publish NeuralSignal message to signal_topic */
#endif

    return 0;
}

int eni_ros2_spin_once(eni_ros2_node_t *node)
{
    if (!node || !node->initialized) return -1;

#ifdef ENI_HAS_ROS2
    /* TODO: rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)); */
#endif

    return 0;
}

void eni_ros2_shutdown(eni_ros2_node_t *node)
{
    if (!node) return;

#ifdef ENI_HAS_ROS2
    /* TODO: Clean up ROS 2 resources */
#endif

    node->running = 0;
    node->initialized = 0;
    printf("[ROS2] Node shutdown\n");
}
