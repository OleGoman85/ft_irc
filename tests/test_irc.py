#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import sys
from irc_tester import IRCTestClient

# красивости
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
    В течение 'timeout' секунд получает строки от сервера.
    Если находит строку, содержащую 'substring', возвращает её.
    Если нет — возвращает None.
    """
    end_time = time.time() + timeout
    received_lines = []
    while time.time() < end_time:
        line = client.receive_line()
        if line is None:
            time.sleep(0.1)
            continue
        received_lines.append(line)
        if substring in line:
            return line
    print(f"{RED}[⚠️] Timeout! Didn't receive expected '{substring}'. Got instead: {received_lines}{RESET}")
    return None

def main():
    HOST = "127.0.0.1"
    PORT = 6667
    PASSWORD = "mypassword"

    print(f"{BLUE}\n🔍 Running IRC tests...\n{RESET}")

    # 1️⃣ Создаём и авторизуем первого клиента (Alisa)
    print_header("Тест регистрации (Alisa)")
    client1 = IRCTestClient(HOST, PORT, timeout=2)
    if not client1.sock:
        print_result(False, "Client1 could not connect.")
        sys.exit(1)

    client1.auth(password=PASSWORD, nick="Alisa", user="Alisa")
    response = wait_for_line_containing(client1, "001", timeout=5.0)
    print_result(response is not None, "Client1 received '001' welcome", response)

    # 2️⃣ Второй клиент (Bob)
    print_header("Тест регистрации (Bob)")
    client2 = IRCTestClient(HOST, PORT, timeout=2)
    if not client2.sock:
        print_result(False, "Client2 could not connect.")
        client1.close()
        sys.exit(1)

    client2.auth(password=PASSWORD, nick="Bob", user="Bob")
    response = wait_for_line_containing(client2, "001", timeout=5.0)
    print_result(response is not None, "Client2 received '001' welcome", response)

    # 3️⃣ Alisa JOIN #test
    print_header("Тест JOIN #test (Alisa)")
    client1.send_line("JOIN #test")
    response = wait_for_line_containing(client1, f":Alisa JOIN #test")
    print_result(response is not None, "Alisa successfully joined #test", response)

    # Bob JOIN #test
    print_header("Тест JOIN #test (Bob)")
    client2.send_line("JOIN #test")
    response = wait_for_line_containing(client2, f":Bob JOIN #test")
    print_result(response is not None, "Bob successfully joined #test", response)

    # 4️⃣ Bob отправляет в канал PRIVMSG, Alisa должна его получить
    print_header("Тест PRIVMSG (Bob -> #test)")
    test_message = "Hello from Bob!"
    client2.send_line(f"PRIVMSG #test :{test_message}")
    response = wait_for_line_containing(client1, test_message, timeout=5.0)
    print_result(response is not None, "Alisa received Bob's PRIVMSG", response)

    # 5️⃣ Alisa пытается кикнуть Bob (KICK #test Bob)
    print_header("Тест KICK (Alisa -> Bob)")
    client1.send_line("KICK #test Bob :Bye!")

    # Bob должен увидеть "KICK #test Bob" или Alisa должна получить ошибку "482" (не оператор)
    response_kick = wait_for_line_containing(client2, "KICK #test Bob", timeout=2.0)
    response_482 = wait_for_line_containing(client1, "482", timeout=2.0)

    if response_kick:
        print_result(True, "Bob received KICK", response_kick)
    elif response_482:
        print_result(True, "Alisa received 'not an operator' error (482)", response_482)
    else:
        print_result(False, "KICK test failed! Bob didn't get kicked, and Alisa didn't get 482.")

    # 6️⃣ Оба клиента выходят (QUIT)
    print_header("Тест QUIT (Alisa и Bob)")
    client1.send_line("QUIT :Bye!")
    client2.send_line("QUIT :Bye!")
    time.sleep(1)  # ждём-с пока сервер обработает выход

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
