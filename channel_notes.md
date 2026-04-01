Subject requirements:
◦ You must have operators and regular users.
◦ Then, you have to implement the commands that are specific to channel
operators:
∗ KICK - Eject a client from the channel
∗ INVITE - Invite a client to a channel
∗ TOPIC - Change or view the channel topic
∗ MODE - Change the channel’s mode:
· i: Set/remove Invite-only channel
· t: Set/remove the restrictions of the TOPIC command to channel
operators
· k: Set/remove the channel key (password)
· o: Give/take channel operator privilege
· l: Set/remove the user limit to channel


NAMING:

- Channels names are strings (beginning with a '&', '#', '+' or '!' character) of length up to fifty (50) characters. Hereafter called "channel prefix"
- Channel names are case insensitive.
- Channel name SHALL NOT contain any spaces (' '), a control G (^G or ASCII 7), a comma (',' which is used as a list item separator by the protocol).  Also, a colon (':') is used as a delimiter for the channel mask.  The exact syntax of a channel name is defined in "IRC Server Protocol" [IRC-SERVER].
-  The use of different prefixes effectively creates four (4) distinct namespaces for channel names.  This is important because of the protocol limitations regarding namespaces (in general).  See section 6.1 (Labels) for more details on these limitations.

SCOPE:
- A channel entity is known by one or more servers on the IRC network.
- A user can only become member of a channel known by the server to
   which the user is directly connected.
- The list of servers which know of the existence of a particular channel MUST be a contiguous part of the IRC network, in order for the messages addressed to the channel to be sent to all the channel members.
- Channels with '&' as prefix are local to the server where they are created.

