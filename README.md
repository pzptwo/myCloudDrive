# myCloudDrive - 云盘系统

基于 Qt5 的 C/S 架构云盘系统，支持用户管理、好友系统、文件管理和即时通讯。

## 功能特性

- **用户系统**：注册、登录、在线状态管理
- **好友系统**：搜索用户、添加/删除好友、刷新好友列表
- **即时通讯**：私聊、群聊
- **文件管理**：创建/删除/重命名文件夹，上传/下载/删除/移动/共享文件
- **自定义协议**：基于 TCP 的 PDU 自定义通讯协议，支持结构化数据传输

## 项目结构

```
review_cloudDrive/
├── TcpServer/                    # 服务端
│   ├── main.cpp                  # 入口
│   ├── protocol.h / protocol.cpp # 通讯协议定义
│   ├── mytcpserver.h / .cpp      # TCP 服务器，管理客户端连接
│   ├── mytcpsocket.h / .cpp      # 单个客户端 Socket 处理（消息收发、文件操作）
│   ├── opedb.h / .cpp            # 数据库操作（SQLite）
│   └── tcpserver.h / .cpp / .ui  # 服务端 UI
│
├── TcpClient/                    # 客户端
│   ├── main.cpp                  # 入口
│   ├── protocol.h / protocol.cpp # 通讯协议定义（与服务器一致）
│   ├── tcpclient.h / .cpp / .ui  # 主窗口（登录/注册）
│   ├── opewidget.h / .cpp        # 主操作界面（好友/图书标签切换）
│   ├── friendlw.h / .cpp         # 好友列表页面
│   ├── online.h / .cpp / .ui     # 在线用户弹窗
│   ├── book.h / .cpp             # 文件管理页面（网盘）
│   ├── sharefile.h / .cpp        # 共享文件选择窗口
│   └── privatechat.h / .cpp / .ui# 私聊窗口
│
└── .gitignore
```

## 环境要求

| 依赖 | 版本 |
|------|------|
| Qt | 5.15.2 |
| 编译器 | MinGW 64-bit |
| C++ 标准 | C++17 |
| 数据库 | SQLite（Qt SQL 模块） |
| Qt 模块 | core, gui, widgets, network, sql |

## 编译与运行

### 服务端

```bash
cd TcpServer
qmake TcpServer.pro
make
./release/TcpServer.exe
```

服务端默认监听 `127.0.0.1:8888`，配置文件位于 `TcpServer/config.qrc` 内的 `server.config`。

### 客户端

```bash
cd TcpClient
qmake TcpClient.pro
make
./debug/TcpClient.exe
```

客户端连接配置位于 `TcpClient/config.qrc` 内的 `client.config`。

## 通讯协议

系统使用自定义 PDU（Protocol Data Unit）基于 TCP 传输：

```cpp
struct PDU {
    uint uiPDULen_;    // PDU 总长度
    uint uiMsgLen_;    // 实际消息长度（caMsg 部分）
    uint uiMsgType_;   // 消息类型（见枚举）
    char caData[64];   // 附加数据（用户名、文件名等）
    int  caMsg[];      // 弹性数组，存放实际消息体
};
```

消息类型定义在 `protocol.h` 中，按功能分为：

| 类别 | 消息类型 |
|------|---------|
| 注册/登录 | `REGISTER`, `LOGIN` |
| 在线用户 | `ALL_ONLINE`, `SEARCH_USER` |
| 好友管理 | `ADD_USER`, `FLUSH_FRIEND`, `DEL_FRIEND` |
| 即时通讯 | `PRIVATE_CHAT`, `GROUP_CHAT` |
| 文件管理 | `CREATE_DIR`, `FLUSH_DIR`, `DEL_DIR`, `RENAME_FILE`, `ENTRY_DIR`, `UPLOAD_FILE`, `DEL_FILE`, `DOWNLOAD_FILE`, `SHARE_FILE`, `MOVE_FILE` |

## 使用说明

1. 先启动**服务端** `TcpServer.exe`
2. 启动**客户端** `TcpClient.exe`，注册账号后登录
3. 登录后进入主界面，左侧为功能标签（好友 / 图书）
4. **好友**页面：搜索用户、添加好友、刷新列表、私聊、群聊
5. **图书**页面：文件管理操作，点击"共享文件"可分享给好友

## 已知问题

- 服务端 `mytcpsocket.cpp` 中 `strName_` 在登录时赋值，一个 socket 对应一个用户名，依赖于连接不会复用

## 已修复

- **共享文件弹窗无好友选项**: `Book::shareFile()` 中新增 `friendlw.flushFriend()` 调用，并在 `recvMsg()` 的 `FLUSH_FRIEND_RESPONSE` 处理中同步刷新共享弹窗的好友复选框
- **caData 传错发送者名字**: `shareFile::selectOk()` 中将 `sprintf(pdu->caData,...,strShareFileName,...)` 修正为使用 `strsharetooneName`（当前登录用户）

## 数据库

使用 SQLite，数据库文件在服务端运行目录下，主要表结构：

```sql
-- 用户表
CREATE TABLE userInfo (name TEXT, pwd TEXT, online INTEGER);

-- 好友关系表
CREATE TABLE friend (id INTEGER, friendId INTEGER);
```
