#ifndef LWIP_PING_H
#define LWIP_PING_H


#include "lwip/ip_addr.h"
#include "err.h"

#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif
#ifdef __cplusplus
extern "C"
{
#endif
extern void ping_init(const ip_addr_t* ping_addr);
#if PING_USE_SOCKETS
extern void ping_recv(int s, uint32_t ping_time);
extern err_t ping_send(int s, const ip_addr_t *addr);
#endif

#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#ifdef __cplusplus
}
#endif
#endif /* LWIP_PING_H */
