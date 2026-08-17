# 云盘项目 Bug 修复记录

> 修复时间：2026年8月4日  
> 项目：myCloudDrive（云盘系统）  
> 修复人：bob + AI 辅助

---

## 目录

1. [Bug 1：共享文件夹拷贝全部失败](#bug-1共享文件夹拷贝全部失败)
2. [Bug 2：服务端可能因网络数据不完整而崩溃](#bug-2服务端可能因网络数据不完整而崩溃)
3. [Bug 3：数据库路径写死了你电脑的路径](#bug-3数据库路径写死了你电脑的路径)
4. [Bug 4：多处内存泄漏，服务端运行久了会卡死](#bug-4多处内存泄漏服务端运行久了会卡死)

---

## Bug 1：共享文件夹拷贝全部失败

**Commit：** `064f53b`  
**文件：** `TcpServer/mytcpsocket.cpp`

### 现象

用户 A 共享一个**文件夹**给用户 B，B 接收后，文件全部拷贝失败，什么提示也没有。

但是共享**单个文件**是正常的。

### 原因

看一下出问题的函数 `copyDir()`：

```cpp
void myTcpSocket::copyDir(QString srcPath, QString desPath)
{
    // ...
    QString srcTmp;     // 源路径，初始是空字符串 ""
    QString desTmp;     // 目标路径，初始是空字符串 ""
    
    for (int i = 0; i < fileInfoList.size(); i++)
    {
        if (fileInfoList[i].isFile())
        {
            srcTmp = srcPath + '/' + fileInfoList[i].fileName();
            desTmp = desTmp + '/' + fileInfoList[i].fileName();  // ❌ 这里用的是 desTmp！
            QFile::copy(srcTmp, desTmp);
        }
    }
}
```

问题出在 `desTmp = desTmp + '/' + ...` 这一行。

**打个比方：** 超市有两种购物篮：
- `srcPath`（源路径）和 `desPath`（目标路径）是你**一开始拿到的两个正确地址**
- `srcTmp` 和 `desTmp` 是你在里面**现拼的新地址**

`srcTmp` 拼对了——它是 `srcPath` + 文件名  
**但 `desTmp` 拼错了——它写成了 `desTmp` + 文件名**

`desTmp` 一开始是空的 `""`，所以：
- 第1个文件拼出来是 `"/文件1.txt"` → 拷贝到了 C 盘根目录
- 第2个文件拼出来是 `"/文件1.txt/文件2.txt"` → 找不到这个路径，直接失败
- 后面的全都失败了

**本质原因：** 把 `desPath`（参数里的目标目录）写成了 `desTmp`（自己拼的变量）。

### 修复

```cpp
// 修改前
desTmp = desTmp + '/' + fileInfoList[i].fileName();

// 修改后
desTmp = desPath + '/' + fileInfoList[i].fileName();
```

`desPath` 是调用时传进来的正确目标路径，用它做前缀就行了。

共修改了 2 处（文件分支和文件夹分支各一处）。

---

## Bug 2：服务端可能因网络数据不完整而崩溃

**Commit：** `6a2d26e`  
**文件：** `TcpServer/mytcpsocket.cpp`、`TcpClient/tcpclient.cpp`

### 现象

服务端偶尔会**无故崩溃退出**，没有明显规律。

### 背景知识（重要！）

网络传输数据不是"一个消息一个包"这样发的。TCP 是**流式协议**，像水管流水一样：

```
实际发送：  [消息A的头部..........................]
TCP 收到：  [前4字][后3字......................................]
               ↑ 只到了半个消息头！
```

两种常见情况：
1. **半包**：一个完整的消息分好几次才收完，当前只到了一半
2. **粘包**：多个消息粘在一起，一次全到了

### 原因

原来的代码：

```cpp
void myTcpSocket::recvMsg()
{
    uint uiPDUlen = 0;
    this->read((char*)&uiPDUlen, sizeof(uint));  // ❌ 不管来了多少字节，硬读4个字节！
    
    uint uiMsgLen = uiPDUlen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char *)pdu + sizeof(uint), uiPDUlen - sizeof(uint));  // ❌ 又硬读！
    
    switch (pdu->uiMsgType_)  // ← 如果只收到了半个消息头，这里读到的是随机值
    {
        case 注册:
        case 登录:
        // ...如果 uiMsgType_ 是随机数，switch 会跳到一个不存在的分支
        // 程序直接崩溃！
    }
}
```

如果只到了 2 个字节就触发了信号，那 `read((char*)&uiPDUlen, 4)` 会读到什么？
- 前 2 字节是真的数据
- 后 2 字节是**未初始化的内存垃圾**

于是 `uiPDUlen` 可能是几千万，`mkPDU(几千万)` 申请超大内存，直接挂掉。

### 修复

加两个保护：

1. **用 `peek()` 先偷看，不消费数据**  
2. **检查数据够不够，不够就等下次再来**

```cpp
void myTcpSocket::recvMsg()
{
    while (this->bytesAvailable() >= sizeof(uint))  // 至少有4字节才进循环
    {
        uint uiPDUlen = 0;
        this->peek((char*)&uiPDUlen, sizeof(uint));  // peek：偷看，不消费
        
        if (this->bytesAvailable() < (int)uiPDUlen)
        {
            return;  // 消息体还没到齐，等下次信号再来
        }
        
        // 确认数据够了，正式读取
        this->read((char*)&uiPDUlen, sizeof(uint));
        // ... 正常处理 ...
    }
}
```

**用 `while` + `peek` 的好处：**
- 半包时：`return` 等下次信号，不会崩溃
- 粘包时：`while` 循环一次把多个消息都处理完

客户端和服务端各改了一处，逻辑一样。

---

## Bug 3：数据库路径写死了你电脑的路径

**Commit：** `625caef`  
**文件：** `TcpServer/opedb.cpp`

### 现象

代码在别人电脑上跑不起来，连数据库都打不开。

### 原因

```cpp
db_.setDatabaseName("C:\\Users\\bob\\Desktop\\myCloudDrive\\review_cloudDrive\\TcpServer\\cloudDrive.db");
```

路径里写了 `bob`（你的用户名）、`Desktop\myCloudDrive\review_cloudDrive`（你电脑上的具体目录）。换一台电脑、换个用户名、换个目录位置，这个路径就不存在了。

### 修复

```cpp
// 修改前
db_.setDatabaseName("C:\\Users\\bob\\Desktop\\myCloudDrive\\review_cloudDrive\\TcpServer\\cloudDrive.db");

// 修改后
db_.setDatabaseName("cloudDrive.db");
```

`cloudDrive.db` 和程序在同一个目录下，运行时只要工作目录对了，自然能找到。全项目其他地方（如用户文件夹 `./用户名`）用的也都是相对路径，保持一致。

---

## Bug 4：多处内存泄漏，服务端运行久了会卡死

**Commit：** `2c61b66`  
**文件：** `TcpServer/mytcpsocket.cpp`

### 背景知识

C++ 中，用 `new` 分配的内存，必须用 `delete` 或 `delete[]` 归还。如果你只借不还，内存就会慢慢被占满——这就是**内存泄漏**。

打个比方：图书馆每次有人借书，你复印一本给他，但不收回。一天几千人借书，复印纸迟早用完，图书馆就瘫痪了。

### 原因

在多个 `case` 分支中，用 `new char[长度]` 申请了堆内存，但 `break` 退出前忘记释放：

```cpp
case ENUM_MSG_TYPE_DEL_DIR_RESPEST:
{
    char *pPath = new char[pdu->uiMsgLen_];  // 申请了一块内存
    
    // ... 使用 pPath ...
    
    free(respdu);
    respdu = nullptr;
    break;  // ❌ pPath 没释放！这块内存永远回不去了
}
```

每次有人删除文件夹，就泄露一小块内存。服务端可能要运行几个月，日积月累就会把内存吃光。

共 5 个 case 中有这个问题：
- `FLUSH_DIR`（刷新目录）
- `DEL_DIR`（删除目录）
- `RENAME_FILE`（重命名文件）
- `ENTRY_DIR`（进入目录）
- `DEL_FILE`（删除文件）

另外 `MOVE_FILE`（移动文件）有 2 个 `new char[]` 也没放。

### 修复

在每个 `break` 前加上 `delete[]`：

```cpp
case ENUM_MSG_TYPE_DEL_DIR_RESPEST:
{
    char *pPath = new char[pdu->uiMsgLen_];
    
    // ... 使用 pPath ...
    
    write((char *)respdu, respdu->uiPDULen_);
    free(respdu);
    respdu = nullptr;
    delete[] pPath;     // ← 加这一行
    pPath = nullptr;    // ← 好习惯：置空防止误用
    break;
}
```

**为什么用 `delete[]` 而不是 `delete`？**  
因为 `new char[长度]` 用的是数组分配，必须用带中括号的 `delete[]` 释放，否则还会造成部分泄漏。

---

## 总结

| Bug | 严重程度 | 一句话说明 |
|-----|---------|-----------|
| Bug 1 | 致命 | 变量名写错了，`desTmp` 写成 `desPath` |
| Bug 2 | 致命 | 网络数据没到齐就硬读，读到垃圾数据崩溃 |
| Bug 3 | 中等 | 路径写死了绝对路径，换电脑就跑不了 |
| Bug 4 | 严重 | 借内存不还，服务端长期运行会内存耗尽 |

---

## Git 提交记录

```
064f53b fix: copyDir() 目标路径构建错误，修复文件夹共享拷贝全部失败的问题
6a2d26e fix: recvMsg() 未校验 bytesAvailable()，TCP 半包可导致崩溃
625caef fix: 数据库路径硬编码为绝对路径，改为相对路径提升可移植性
2c61b66 fix: 多个 case 中 new char[] 未 delete[]，修复长期运行内存泄漏
```

---

*文档生成于 2026年8月4日*
