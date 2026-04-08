# websocket-echo

Echoes every message a peer sends, after greeting them with `welcome` on
connect. Demonstrates the three WebSocket exports a flare can implement:
`ws_open`, `ws_message`, and `ws_close`.

Configure the flare with `kind = "websocket"` in the dashboard.
