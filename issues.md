# 📌 Вылезшие проблемы

## 1️⃣ Сделать `_password`, `_clients`, `_channels` приватными (Server.hpp)

**Проблема:**
Сейчас эти поля объявлены как `public` в файле `Server.hpp`, но они должны быть `private`, так как не должны изменяться напрямую извне класса `Server`.

Я добавила геттеры, которыми теперь можно пользоваться вместо прямого обращения к этим полям:

```cpp
const std::string& getPassword() const;
std::map<int, std::unique_ptr<Client>>& getClients();
std::map<std::string, Channel>& getChannels();
```

Теперь нужно:

1. **Сделать `_password`, `_clients`, `_channels` приватными в файле `Server.hpp`**.
2. **Заменить все вызовы в командах** (например, в файлах `Nick.cpp`, `Join.cpp`, `Kick.cpp` и т. д.), которые напрямую обращаются к этим полям, на геттеры.

### 🔧 **Пример исправления**

#### ❌ **Было (например, в `Nick.cpp`)**:

```cpp
server->_clients[fd]->setNickname(nickname);
```

#### ✅ **Стало:**

```cpp
server->getClients().at(fd)->setNickname(nickname);
```

