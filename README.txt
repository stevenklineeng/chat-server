Hello! I am Steven Kline, and this is a code that I created that functions as a chat server. I use sockets and threads in the C programming language to 
implement this. I hope you like it!

COMPILE:
  server: gcc chat_server.c -lpthread -o chat_server
  client: gcc chat_client.c -lpthread -o chat_client

RUN:
  server: ./chat_server <port_number>
  client: ./chat_client <IP_address> <port_number> <username>
