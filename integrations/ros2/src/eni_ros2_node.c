// SPDX-License-Identifier: MIT
// eNI ROS 2 Node — Neural intent publisher and feedback subscriber

#include <stdio.h>
#include <string.h>

#ifdef ENI_HAS_ROS2
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>
#include <std_msgs/msg/float32_multi_array.h>
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
#ifdef ENI_HAS_ROS2
    rcl_allocator_t        allocator;
    rclc_support_t         support;
    rcl_node_t             node;
    rcl_publisher_t        intent_pub;
    rcl_publisher_t        signal_pub;
    rcl_subscription_t     feedback_sub;
    rclc_executor_t        executor;
    std_msgs__msg__String  feedback_msg;
#endif
} eni_ros2_node_t;

#ifdef ENI_HAS_ROS2
static void feedback_callback(const void *msg_in)
{
    const std_msgs__msg__String *msg = (const std_msgs__msg__String *)msg_in;
    if (msg && msg->data.data) {
        printf("[ROS2] Feedback received: %s\n", msg->data.data);
    }
}
#endif

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
    node->allocator = rcl_get_default_allocator();

    /* Initialize RCL support */
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    rcl_ret_t rc = rcl_init_options_init(&init_options, node->allocator);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to init options: %d\n", (int)rc);
        return -1;
    }

    rc = rclc_support_init(&node->support, 0, NULL, &init_options);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to init support: %d\n", (int)rc);
        return -1;
    }

    /* Create node */
    rc = rclc_node_init_default(&node->node, config->node_name, "", &node->support);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to create node '%s': %d\n", config->node_name, (int)rc);
        return -1;
    }

    /* Create intent publisher (String messages) */
    rc = rclc_publisher_init_default(
        &node->intent_pub, &node->node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        config->intent_topic);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to create intent publisher: %d\n", (int)rc);
        return -1;
    }

    /* Create signal publisher (Float32MultiArray messages) */
    rc = rclc_publisher_init_default(
        &node->signal_pub, &node->node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray),
        config->signal_topic);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to create signal publisher: %d\n", (int)rc);
        return -1;
    }

    /* Create feedback subscriber */
    rc = rclc_subscription_init_default(
        &node->feedback_sub, &node->node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        config->feedback_topic);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to create feedback subscriber: %d\n", (int)rc);
        return -1;
    }

    /* Initialize executor with 1 subscription */
    rc = rclc_executor_init(&node->executor, &node->support.context, 1, &node->allocator);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to init executor: %d\n", (int)rc);
        return -1;
    }

    std_msgs__msg__String__init(&node->feedback_msg);
    rc = rclc_executor_add_subscription(
        &node->executor, &node->feedback_sub,
        &node->feedback_msg, &feedback_callback, ON_NEW_DATA);
    if (rc != RCL_RET_OK) {
        printf("[ROS2] Failed to add subscription to executor: %d\n", (int)rc);
        return -1;
    }
#endif

    node->initialized = 1;
    node->running = 1;
    printf("[ROS2] Node '%s' initialized\n", node->config.node_name);
    printf("[ROS2]   intent topic:   %s\n", node->config.intent_topic);
    printf("[ROS2]   signal topic:   %s\n", node->config.signal_topic);
    printf("[ROS2]   feedback topic: %s\n", node->config.feedback_topic);
    printf("[ROS2]   rate:           %d Hz\n", node->config.publish_rate_hz);
    return 0;
}

int eni_ros2_publish_intent(eni_ros2_node_t *node, const char *intent, float confidence, int class_id)
{
    if (!node || !node->initialized || !intent) return -1;

#ifdef ENI_HAS_ROS2
    /* Build intent message as JSON string */
    std_msgs__msg__String msg;
    std_msgs__msg__String__init(&msg);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "{\"intent\":\"%s\",\"confidence\":%.3f,\"class_id\":%d,\"seq\":%d}",
             intent, confidence, class_id, node->publish_count + 1);

    rosidl_runtime_c__String__assign(&msg.data, buf);
    rcl_ret_t rc = rcl_publish(&node->intent_pub, &msg, NULL);
    std_msgs__msg__String__fini(&msg);

    if (rc != RCL_RET_OK) {
        printf("[ROS2] Publish intent failed: %d\n", (int)rc);
        return -1;
    }
#endif

    node->publish_count++;
    printf("[ROS2] Published intent: %s (%.2f, class=%d) #%d\n",
           intent, confidence, class_id, node->publish_count);
    return 0;
}

int eni_ros2_publish_signal(eni_ros2_node_t *node, const float *samples, int num_channels, uint64_t timestamp)
{
    if (!node || !node->initialized || !samples) return -1;

#ifdef ENI_HAS_ROS2
    std_msgs__msg__Float32MultiArray msg;
    std_msgs__msg__Float32MultiArray__init(&msg);

    /* Set data */
    rosidl_runtime_c__float__Sequence__init(&msg.data, (size_t)num_channels);
    for (int i = 0; i < num_channels; i++) {
        msg.data.data[i] = samples[i];
    }

    rcl_ret_t rc = rcl_publish(&node->signal_pub, &msg, NULL);
    std_msgs__msg__Float32MultiArray__fini(&msg);

    if (rc != RCL_RET_OK) {
        printf("[ROS2] Publish signal failed: %d\n", (int)rc);
        return -1;
    }
#else
    (void)num_channels;
    (void)timestamp;
#endif

    return 0;
}

int eni_ros2_spin_once(eni_ros2_node_t *node)
{
    if (!node || !node->initialized) return -1;

#ifdef ENI_HAS_ROS2
    /* Process pending callbacks with 10ms timeout */
    rclc_executor_spin_some(&node->executor, RCL_MS_TO_NS(10));
#endif

    return 0;
}

void eni_ros2_shutdown(eni_ros2_node_t *node)
{
    if (!node) return;

#ifdef ENI_HAS_ROS2
    /* Clean up ROS 2 resources in reverse order */
    rclc_executor_fini(&node->executor);
    rcl_subscription_fini(&node->feedback_sub, &node->node);
    rcl_publisher_fini(&node->signal_pub, &node->node);
    rcl_publisher_fini(&node->intent_pub, &node->node);
    rcl_node_fini(&node->node);
    rclc_support_fini(&node->support);
    std_msgs__msg__String__fini(&node->feedback_msg);
#endif

    node->running = 0;
    node->initialized = 0;
    printf("[ROS2] Node '%s' shutdown (published %d intents)\n",
           node->config.node_name, node->publish_count);
}
