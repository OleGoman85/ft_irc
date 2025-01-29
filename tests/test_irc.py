#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import sys
from irc_tester import IRCTestClient 


GREEN = "\033[1;32m"
RED = "\033[1;31m"
YELLOW = "\033[1;33m"
BLUE = "\033[1;34m"
RESET = "\033[0m"

def print_header(text):
    print(f"\n{YELLOW}{'='*50}\n📌 {text}\n{'='*50}{RESET}")

def print_result(success, message, received=""):
    if success:
        print(f"{GREEN}[✅] {message} PASSED!{RESET}")
    else:
        print(f"{RED}[❌] {message} FAILED!{RESET}")
        if received:
            print(f"{RED}     ⏪ Server response: {received}{RESET}")

def wait_for_line_containing(client: IRCTestClient, substring: str, timeout: float = 3.0) -> str:
    """
    В течение 'timeout' секунд пытается получить строку, содержащую substring.
    Если находим — возвращаем строку, иначе возвращаем None.
    """
    end_time = time.time() + timeout
    lines_seen = []
    while time.time() < end_time:
        line = client.receive_line()
        if line is None:
            # Нет новых данных (таймаут чтения) или разрыв. Подождём чуть-чуть
            time.sleep(0.1)
            continue
        lines_seen.append(line)
        if substring in line:
            return line
    print(f"{RED}[⚠️] Timeout! Didn't receive expected '{substring}'. Got instead: {lines_seen}{RESET}")
    return None

def main():
    HOST = "127.0.0.1"
    PORT = 6667
    PASSWORD = "mypassword"

    print(f"{BLUE}\n🔍 Running IRC tests...\n{RESET}")

    # 1) Первый клиент (Alisa)
    print_header("Тест регистрации (Alisa)")
    client1 = IRCTestClient(HOST, PORT, timeout=2)
    if not client1.sock:
        print_result(False, "Client1 could not connect.")
        sys.exit(1)

    # Авторизуемся
    client1.auth(password=PASSWORD, nick="Alisa", user="Alisa")
    resp = wait_for_line_containing(client1, "001", timeout=5.0)
    print_result(resp is not None, "Alisa received '001' welcome", resp)

    # 2) Второй клиент (Bob)
    print_header("Тест регистрации (Bob)")
    client2 = IRCTestClient(HOST, PORT, timeout=2)
    if not client2.sock:
        print_result(False, "Client2 could not connect.")
        client1.close()
        sys.exit(1)

    client2.auth(password=PASSWORD, nick="Bob", user="Bob")
    resp = wait_for_line_containing(client2, "001", timeout=5.0)
    print_result(resp is not None, "Bob received '001' welcome", resp)

    # 3) Alisa JOIN #test
    print_header("Тест JOIN #test (Alisa)")
    client1.send_line("JOIN #test")
    # IRC обычно присылает что-то вроде :Alisa!~host JOIN :#test
    # Для упрощения ищем просто "JOIN #test"
    resp = wait_for_line_containing(client1, "JOIN #test", timeout=5.0)
    print_result(resp is not None, "Alisa successfully joined #test", resp)

    # Bob JOIN #test
    print_header("Тест JOIN #test (Bob)")
    client2.send_line("JOIN #test")
    resp = wait_for_line_containing(client2, "JOIN #test", timeout=5.0)
    print_result(resp is not None, "Bob successfully joined #test", resp)

    # 4) PRIVMSG
    print_header("Тест PRIVMSG (Bob -> #test)")
    test_msg = "Hello from Bob!"
    client2.send_line(f"PRIVMSG #test :{test_msg}")
    # Alisa должна увидеть в одной из строк этот test_msg
    resp = wait_for_line_containing(client1, test_msg, timeout=5.0)
    print_result(resp is not None, "Alisa received Bob's PRIVMSG", resp)

    # 5) KICK
    print_header("Тест KICK (Alisa -> Bob)")
    client1.send_line("KICK #test Bob :Bye!")
    # Bob может увидеть "KICK #test Bob" если Alisa оп, 
    # иначе Alisa получит "482 #test :You're not a channel operator"
    resp_kick = wait_for_line_containing(client2, "KICK #test Bob", timeout=2.0)
    resp_482 = wait_for_line_containing(client1, "482", timeout=2.0)
    if resp_kick:
        print_result(True, "Bob received KICK", resp_kick)
    elif resp_482:
        print_result(True, "Alisa received 'not an operator' error (482)", resp_482)
    else:
        print_result(False, "KICK test failed! No KICK, no 482 message.")

    # 6) MODE tests
    print_header("MODE tests on #modeTest")

    # Alisa joins #modeTest. She should be operator automatically as the first user.
    client1.send_line("JOIN #modeTest")
    resp = wait_for_line_containing(client1, "JOIN #modeTest", timeout=5.0)
    print_result(resp is not None, "Alisa joined #modeTest", resp)

    # (A) +i test
    print_header("MODE #modeTest +i (invite-only)")
    client1.send_line("MODE #modeTest +i")
    # We expect a broadcast "MODE #modeTest +i" or similar
    mode_resp = wait_for_line_containing(client1, "MODE #modeTest +i", timeout=3.0)
    if not mode_resp:
        # Maybe the server only sends it to other members. Let's also check client2
        mode_resp2 = wait_for_line_containing(client2, "MODE #modeTest +i", timeout=3.0)
        print_result(mode_resp2 is not None, "Set +i on #modeTest", mode_resp2)
    else:
        print_result(True, "Set +i on #modeTest", mode_resp)

    # Bob tries to JOIN #modeTest -> should fail with "473 #modeTest"
    print_header("Bob tries to join invite-only #modeTest -> expected 473")
    client2.send_line("JOIN #modeTest")
    err_resp = wait_for_line_containing(client2, "473 #modeTest", timeout=3.0)
    print_result(err_resp is not None, "Bob got 473 (invite-only error)", err_resp)

    # (A) -i test
    print_header("MODE #modeTest -i (remove invite-only)")
    client1.send_line("MODE #modeTest -i")
    mode_resp = wait_for_line_containing(client2, "MODE #modeTest -i", timeout=3.0)
    print_result(mode_resp is not None, "Unset +i on #modeTest", mode_resp)

    # Now Bob can join
    print_header("Bob joins #modeTest (should succeed now)")
    client2.send_line("JOIN #modeTest")
    join_resp = wait_for_line_containing(client2, "JOIN #modeTest", timeout=5.0)
    print_result(join_resp is not None, "Bob joined #modeTest successfully", join_resp)

    # (B) +k <secretKey> test
    print_header("MODE #modeTest +k secretKey")
    client1.send_line("MODE #modeTest +k secretKey")
    mode_resp = wait_for_line_containing(client2, "MODE #modeTest +k secretKey", timeout=3.0)
    print_result(mode_resp is not None, "Set +k on #modeTest (secretKey)", mode_resp)

    # Bob PART so we can test re-join with/without key
    print_header("Bob PART #modeTest")
    client2.send_line("PART #modeTest :I go out")
    part_resp = wait_for_line_containing(client2, "PART #modeTest", timeout=3.0)
    print_result(part_resp is not None, "Bob parted #modeTest", part_resp)

    # Bob tries join #modeTest without the key -> expect "475 #modeTest"
    print_header("Bob tries to join #modeTest without key -> expected 475")
    client2.send_line("JOIN #modeTest")
    err_resp = wait_for_line_containing(client2, "475 #modeTest", timeout=3.0)
    print_result(err_resp is not None, "Bob got 475 (bad key)", err_resp)

    # Bob tries join with key
    print_header("Bob joins #modeTest with key=secretKey -> expect success")
    client2.send_line("JOIN #modeTest secretKey")
    join_resp = wait_for_line_containing(client2, "JOIN #modeTest", timeout=5.0)
    print_result(join_resp is not None, "Bob joined #modeTest with correct key", join_resp)


    # 6) QUIT
    print_header("Тест QUIT (Alisa & Bob)")
    client1.send_line("QUIT :Bye!")
    client2.send_line("QUIT :Bye!")
    time.sleep(1)
    client1.close()
    client2.close()

    print(f"{BLUE}\n🎯 All tests completed!{RESET}\n")
    sys.exit(0)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[INTERRUPT] Tests aborted by user.")
        sys.exit(1)
    except Exception as e:
        print(f"[ERROR] Unexpected exception: {e}")
        sys.exit(1)
