#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import subprocess
import time
import test_mode 
import test_invite
import test_join


BLUE = "\033[1;34m"
RED = "\033[1;31m"
GOLD = "\033[1;33m"
RESET = "\033[0m"


SERVER_EXEC = "./ircserv"
SERVER_PORT = 6667
SERVER_PASSWORD = "password"

def print_banner():
    print(f"""
{GOLD}

 ,ggg, ,ggg,_,ggg,                                                            
dP""Y8dP""Y88P""Y8b                                           I8              
Yb, `88'  `88'  `88                                           I8              
 `"  88    88    88                  gg                    88888888           
     88    88    88                  ""                       I8              
     88    88    88    ,gggg,gg      gg   ,ggg,     ,g,       I8    gg     gg 
     88    88    88   dP"  "Y8I      8I  i8" "8i   ,8'8,      I8    I8     8I 
     88    88    88  i8'    ,8I     ,8I  I8, ,8I  ,8'  Yb    ,I8,   I8,   ,8I 
     88    88    Y8,,d8,   ,d8b,  _,d8I  `YbadP' ,8'_   8)  ,d88b, ,d8b, ,d8I 
     88    88    `Y8P"Y8888P"`Y8888P"888888P"Y888P' "YY8P8P88P""Y88P""Y88P"888
                                   ,d8I'                                 ,d8I'
                                 ,dP'8I                                ,dP'8I 
                                ,8"  8I                               ,8"  8I 
                                I8   8I                               I8   8I 
                                `8, ,8I                               `8, ,8I 
                                 `Y8P"                                 `Y8P"  

 {RESET}

👑  WELCOME TO {GOLD}MAJESTY{RESET} – HER ROYAL TORTURE MACHINE 👑
""")

def start_server():
    """Запускает IRC-сервер как subprocess."""
    proc = subprocess.Popen([SERVER_EXEC, str(SERVER_PORT), SERVER_PASSWORD],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    time.sleep(1)
    return proc

def stop_server(proc):
    proc.terminate()
    proc.wait()

def main():
    available_tests = {
        "mode": test_mode.run_tests,
        "invite": test_invite.run_tests, 
        "join": test_join.run_tests,
        # присобачиваем новые тесты сюда
    }
    print_banner()
    if len(sys.argv) > 1:
        test_name = sys.argv[1].lower()
        if test_name in available_tests:
            print(f"{BLUE}\n🔍 Запуск тестов для: {test_name.upper()}...{RESET}\n")
            server_proc = start_server()
            try:
                available_tests[test_name]() 
            finally:
                stop_server(server_proc)
            return 

        else:
            print(f"❌ Ошибка: тест '{test_name}' не найден. Доступные тесты: {', '.join(available_tests.keys())}")
            sys.exit(1)

    print(f"{BLUE}\n🔍 Запуск всех тестов...{RESET}\n")
    server_proc = start_server()
    try:
        for test_func in available_tests.values():
            test_func()
    finally:
        stop_server(server_proc)

    print(f"{GOLD}\n👑 Все испытания окончены! Достойные выжили... {RESET}\n")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[INTERRUPT] Majesty прервала пытки.")
        sys.exit(1)
    except Exception as e:
        print(f"[ERROR] Majesty столкнулась с неожиданным препятствием: {e}")
        sys.exit(1)
