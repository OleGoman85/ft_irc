

# **Implementation of Basic IRC Commands**

## **Starting the Server and Connecting a Client**
1. Run the server:
   ```bash
   ./ircserv 6667 <correct_password>
   ```
2. Connect via netcat:
   ```bash
   nc 127.0.0.1 6667
   ```

## **IRC Commands**

✅ **PASS** – Client authentication  
   ```bash
   PASS <correct_password>
   ```

✅ **NICK** – Set the client’s nickname  
   ```bash
   NICK <name>
   ```

✅ **USER** – Set the client’s username  
   ```bash
   USER oleg 0 * :Oleg
   ```
   - `<username>` – the username (used for authentication)  
   - `<mode>` – usually `0` (rarely used)  
   - `<unused>` – an `*` symbol, ignored by the server  
   - `:<real name>` – the user’s real (full) name  

✅ **JOIN** – Join a channel  
   ```bash
   JOIN #testchannel
   ```
   (You must have completed `PASS`, `NICK`, and `USER` beforehand)

✅ **PRIVMSG** – Send messages to users or channels  
   ```bash
   PRIVMSG User2 :Hello, User2!
   PRIVMSG #testchannel :Hello!
   ```

✅ **QUIT** – Properly disconnect from the server  
   ```bash
   QUIT
   ```
   (Requires `PASS`, `NICK`, `USER`, and `JOIN` to be set)

✅ **KICK** – Kick a client from a channel  
   ```bash
   KICK #testchannel <name> :reason
   ```
   (Requires `PASS`, `NICK`, `USER`, and `JOIN`)

✅ **INVITE** – Invite a client to a channel  
   ```bash
   INVITE User2 #testchannel
   ```

✅ **TOPIC** – Set or view a channel’s topic  
   ```bash
   TOPIC #testchannel :New Channel Topic  // set a new topic
   TOPIC #testchannel                     // view the current topic
   ```

✅ **MODE** – Change channel modes  
   ```bash
   MODE #channel +i            // set the channel to invite-only
   MODE #channel +t            // only ops can set the topic
   MODE #channel +k secretkey  // set a channel password
   MODE #channel -k            // remove the password
   MODE #channel +l 10         // set a user limit (10)
   MODE #channel -l            // remove the user limit
   MODE #channel +o someNick   // grant operator privileges
   ```