#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket

class IRCTestClient:
    """
    Класс, представляющий простого IRC-клиента, 
    умеет подключаться к серверу, отправлять/принимать строки.
    """
    def __init__(self, host="127.0.0.1", port=6667, timeout=2):
        """
        Создаёт клиентский сокет и пытается подключиться к IRC-серверу.
        """
        self.host = host
        self.port = port
        self.sock = None
        self._read_buffer = b""  # буфер для накопления данных между вызовами receive_line

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(timeout)
            self.sock.connect((host, port))
            print(f"[✅] Connected to {host}:{port}")
        except socket.error as e:
            print(f"[❌] Connection failed: {e}")
            self.sock = None

    def send_line(self, message: str):
        """
        Отправляет строку с \r\n, как требует IRC-протокол.
        Пример: "JOIN #channel" => отправит "JOIN #channel\r\n" .
        """
        if not self.sock:
            print("[⚠️] Not connected, cannot send data.")
            return
        data = (message + "\r\n").encode("utf-8")
        try:
            self.sock.sendall(data)
            print(f"[➡️] Sent: {message}")
        except socket.error as e:
            print(f"[❌] Send failed: {e}")
            self.close()

    def receive_line(self):
        """
        Читает ОДНУ строку (до \n). Убирает завершающее \r\n.
        
        Возвращает строку (str) без завершающих \r\n, 
        либо None, если соединение закрыто или произошла ошибка/таймаут.
        """
        if not self.sock:
            return None

        while True:
            # Проверяем, есть ли в буфере признак конца строки
            if b"\n" in self._read_buffer:
                line, sep, rest = self._read_buffer.partition(b"\n")
                self._read_buffer = rest
                # Убираем \r в конце, если есть
                line = line.rstrip(b"\r")
                decoded = line.decode("utf-8", errors="replace")
                print(f"[⬅️] Received line: {decoded}")
                return decoded

            # Иначе дочитываем из сокета порцию
            try:
                chunk = self.sock.recv(1024)
                if not chunk:
                    print("[🔴] Server closed connection.")
                    self.close()
                    return None
                self._read_buffer += chunk
            except socket.timeout:
                # Нет данных до таймаута — вернём None
                return None
            except socket.error as e:
                print(f"[❌] Receive failed: {e}")
                self.close()
                return None

    def auth(self, password: str, nick: str, user: str):
        """
        Упрощённая авторизация: отправляет PASS, NICK, USER по очереди.
        """
        if password:
            self.send_line(f"PASS {password}")
        self.send_line(f"NICK {nick}")
        # FORMALLY: USER <username> <hostname> <servername> :<realname>
        self.send_line(f"USER {user} 0 * :RealNameOf{user}")

    def close(self):
        """
        Закрывает соединение.
        """
        if self.sock:
            try:
                self.sock.close()
                print("[🔴] Connection closed")
            except socket.error as e:
                print(f"[⚠️] Error closing socket: {e}")
            finally:
                self.sock = None

    def __del__(self):
        # На всякий случай закрываем при удалении объекта
        self.close()
