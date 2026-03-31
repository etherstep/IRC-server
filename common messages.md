| Command | Params | Parameter Breakdown | Purpose |
| :--- | :---: | :--- | :--- |
| `PASS` | 1 | `<password>` | Sets a connection password. Must be sent before `NICK`/`USER` if the server requires it. |
| `NICK` | 1 | `<nickname>` | Sets or changes the user's nickname. |
| `USER` | 4 | `<user> <mode> <unused> <realname>` | Specifies the username, hostname, servername, and real name. Sent at connection startup. |
| `JOIN` | 1-2 | `<channel>{,<channel>} [<key>{,<key>}]` | Joins one or more channels. Keys (passwords) are optional. |
| `PART` | 1-2 | `<channel>{,<channel>} [<Part Message>]` | Leaves one or more channels, optionally sending a parting message. |
| `PRIVMSG`| 2 | `<receiver>{,<receiver>} <text to be sent>` | Sends a private message to a user or a public message to a channel. |
| `NOTICE` | 2 | `<nickname/channel> <text>` | Similar to `PRIVMSG`, but automated bots/servers will never automatically reply to a `NOTICE` (prevents infinite loops). |
| `PING` | 1-2 | `<server1> [<server2>]` | Tests for an active connection. The server expects a `PONG` response. |
| `PONG` | 1-2 | `<server1> [<server2>]` | The required response to a `PING` message to keep the connection alive. |
| `QUIT` | 0-1 | `[<Quit Message>]` | Disconnects the client from the server, optionally leaving a global quit message. |
| `MODE` | 1+ | `<target> [<modestring> [<mode arguments>...]]` | Changes the modes (permissions/properties) of a user or a channel (e.g., setting someone as a channel operator with `+o`). |
| `TOPIC` | 1-2 | `<channel> [<topic>]` | Retrieves the current channel topic (1 param) or changes the topic (2 params). |
| `KICK` | 2-3 | `<channel> <user> [<comment>]` | Forcibly removes a user from a channel. Usually requires channel operator privileges. |
| `WHOIS` | 1-2 | `[<target>] <mask>` | Queries the server for detailed information about a specific user. |
| `LIST` | 0-2 | `[<channel>{,<channel>} [<server>]]` | Lists all channels and their topics. If channels are specified, it only lists those. |