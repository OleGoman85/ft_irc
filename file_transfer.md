
# **How the File Transfer Command Works**

This IRC server supports sending files between clients using text-based commands. The file transfer process consists of several steps:

1. **Initiate the transfer** — The sender informs the receiver that a file is about to be sent, specifying the file’s size.  
2. **Transfer data** — The file is encoded in base64 and sent in chunks.  
3. **End the transfer** — The sender notifies the server that the transfer is complete.  
4. **Recipient receives the file** — The server assembles all the data and indicates successful transfer or an error (e.g., if the file was not fully sent).

---

## **File Transfer Commands**

### **FILE SEND (Initiate File Transfer)** 
The sender uses this command to notify the server about the start of file transfer.

**Format:**
```irc
FILE SEND <receiver> <filename> <size_in_bytes>
```

**Example:**
```irc
FILE SEND Bob myfile.txt 120
```
- `Bob` — the receiver’s nickname  
- `myfile.txt` — the file name  
- `120` — file size in bytes (on Linux/Mac you can find this via `ls -l myfile.txt`)

---

### **FILE DATA (Transmit File Data)**
After sending `FILE SEND`, the actual file data (base64-encoded) is transmitted in chunks.

**Format:**
```irc
FILE DATA <filename> <base64_encoded_data>
```

**Example:**
```irc
FILE DATA myfile.txt U29tZSBleGFtcGxlIHRleHQ=
```
(This base64 content decodes to `Some example text`)

---

### **FILE END (Complete the Transfer)**
Once all data has been sent, you must tell the server that the transfer is finished.

**Format:**
```irc
FILE END <filename>
```

**Example:**
```irc
FILE END myfile.txt
```

---

## **What Is Base64 and Why Is It Needed?**
Base64 is a way to encode binary files into text format. In IRC, only text-based commands can be sent, so normal binary files must be encoded first.

### **How to Encode a File in Base64?**  
On Linux/Mac:
```bash
base64 myfile.txt
```

### **How to Check the File Size?**  
You need to know the file size in advance to use `FILE SEND`.

On Linux/Mac:
```bash
ls -l myfile.txt
```

---

## **Testing File Transfers**

### **1. Start the Server**
```bash
./ircserv 6667 password
```

### **2. Connect Two Clients**
Open two terminals and connect each to the server using `nc`:

**Sender (Alice):**
```bash
nc 127.0.0.1 6667
```

**Receiver (Bob):**
```bash
nc 127.0.0.1 6667
```

### **3. Authenticate Each Client**
Both clients enter:
```irc
PASS password
NICK Alice
USER Alice 0 * :Alice the Great
```

```irc
PASS password
NICK Bob
USER Bob 0 * :Bob the Receiver
```

### **4. Alice Sends a File**
1. Check the file size (example: `12` bytes):
   ```bash
   ls -l testfile.txt
   ```
2. Encode the file to base64:
   ```bash
   base64 testfile.txt
   ```
   For instance, `Hello, World` becomes `SGVsbG8sIFdvcmxkIQ==`.

3. Send the file:
   ```irc
   FILE SEND Bob testfile.txt 12
   FILE DATA testfile.txt SGVsbG8sIFdvcmxkIQ==
   FILE END testfile.txt
   ```

### **5. Bob Gets a Notification**
Bob should see something like:
```irc
NOTICE Bob :Received file 'testfile.txt' (12 bytes)
```

If fewer bytes are sent than declared, the server should warn:
```irc
NOTICE Alice :File transfer ended, but file is incomplete (9/12)
```

### **6. Decoding the File on Bob’s Side**
```bash
base64 -d received_base64.txt > recovered_file.txt
```

---

## **Expected Server Behavior**

### **Successful Scenario**
- Alice sends `FILE SEND`, followed by `FILE DATA` and `FILE END`.
- Bob receives the file and a confirmation notice.
- If the sizes match, the server says `File transfer completed`.

### **Error: Incomplete File**
- If Alice sends only a portion of the data (`FILE DATA` does not send everything), the server should respond with:
  ```irc
  NOTICE Alice :File transfer ended, but file is incomplete (X/Y bytes)
  ```

### **Error: Invalid Base64**
- If the data is not valid base64, the server should reject the `FILE DATA` command.