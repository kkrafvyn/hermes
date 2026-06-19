#include "hermes_router.h"
#include <string.h>

int hermes_router_init(hermes_router_t *router, hermes_link_t *spi,
                       hermes_link_t *uart, uint8_t gateway_id)
{
    if (!router || !spi || !uart) return -1;
    memset(router, 0, sizeof(*router));
    router->spi_link = spi;
    router->uart_link = uart;
    router->gateway_id = gateway_id;
    return 0;
}

static int forward_frame(hermes_link_t *dst, const hermes_frame_t *frame)
{
    return hermes_link_send(dst, (hermes_msg_type_t)frame->header.type,
                            frame->header.dst_node,
                            frame->payload, frame->header.payload_len);
}

int hermes_router_poll(hermes_router_t *router, uint32_t timeout_ms)
{
    if (!router) return -1;
    hermes_frame_t frame;
    int routed = 0;

    if (hermes_link_recv(router->spi_link, &frame, timeout_ms) == 0) {
        if (frame.header.dst_node == 2)
            forward_frame(router->uart_link, &frame);
        else if (frame.header.dst_node == router->gateway_id)
            hermes_link_handle_frame(router->spi_link, &frame);
        routed++;
    }

    if (hermes_link_recv(router->uart_link, &frame, timeout_ms) == 0) {
        if (frame.header.dst_node == 0)
            forward_frame(router->spi_link, &frame);
        else if (frame.header.dst_node == router->gateway_id)
            hermes_link_handle_frame(router->uart_link, &frame);
        routed++;
    }
    return routed;
}
