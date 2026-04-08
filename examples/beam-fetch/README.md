# beam-fetch

Fetches `https://example.com/` from the edge using the Beam outbound HTTP
client and returns the upstream response JSON to the caller.

Production flares should parse the JSON envelope (`{status, headers, body}`)
and reshape the response. This example forwards it as-is for clarity.
