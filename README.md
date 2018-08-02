# Chat-in-c

Task should be in C code, as a terminal (cli) program under Linux. It must be a TCP IPv4 and IPv6 chat server, or more accurately, chat room. At entering, every user registers himself with a chosen unique for the session nickname.After finishing his session, the nickname is released and another client may use it. Unregistered users cannot send or receive messages.The messages received must contain info of the sender, time of sending and there should be a difference between private messages and messages to all participants. Telnet, netcat (nc) or other tools can be used as clients to the server. The server should not keep history and the new users will only receive messages after the moment they have logged in. The server should be able to work interactively (with messages being shown), or as a daemon. 

It should support the following commands:

`.name <nickname>`

Registering user with chosen <nickname>. If there is a registered user with that name already, this command should give the user an error. The nickname can only contain letters, digits and underscore "_"

`.msg <nickname> <message>`

Sending a message to user within existing nicknames, otherwise, error

`.msg_all`

Sending message too all users

`.list`

Shows list of registered users currently

`.quit`

Leaving the chatroom and terminating the connection

Compile by:

` gcc chat.c -l pthread -o chatserver`

and then use:

`./chatserver`
