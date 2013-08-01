PushService

- [WIP] Common interface
- [DONE] Plugable push providers
- Add doxygen documentation for all public interfaces

APNS

- [DONE] Basic APNS connection
- [DONE] Pool of APNS connections
- [DONE] APNS message wrapper (creates JSON)
- [DONE] Callback system for message delivery. both success and failure.
- [DONE] Connection restart on error
- [DONE] Handle shutdown message properly
- Support for custom field types in APNS message wrapper (not only string but also arrays, objects, etc.)


GCM (WIP)

- [DONE] Basic connection for GCM
- [DONE] Chunked HTTP/1.1 support
- [WIP] Basic HTTP GCM message wrapper
- Parsing of GCM response and invocation of callback with error/success
