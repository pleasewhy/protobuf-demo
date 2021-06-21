

# Protobuf Quick Start

github地址:https://github.com/pleasewhy/protobuf-demo

该教程的环境为centos-tLinux 2.2-集成版

目录：

- [Protobuf Quick Start](#protobuf-quick-start)
  * [1、protobuf 安装](#1-protobuf---)
  * [2、Protocol Buffer语法](#2-protocol-buffer--)
    + [2.1 定义消息类型](#21-------)
    + [2.2 分配Field Numbers](#22---field-numbers)
    + [2.3 指定字段规则](#23-------)
    + [2.4 添加更多的消息](#24--------)
    + [2.5 注释](#25---)
    + [2.6 保留字段](#26-----)
    + [2.7 编译.proto文件](#27---proto--)
    + [2.8 默认值](#28----)
    + [2.9 枚举](#29---)
    + [2.10 使用其他消息类型](#210---------)
  * [3、示例程序](#3-----)

## 1、protobuf 安装

获取protobuf的最新release，可以根据需要下载指定版本和语言。

```shell
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.17.3/protobuf-cpp-3.17.3.tar.gz
# 可以到https://github.com/protocolbuffers/protobuf/releases选择你想要下载的版本
```

编译安装

```shell
tar -zxvf protobuf-cpp-3.17.3.tar.gz
cd protobuf-cpp-3.17.3
./autogen.sh  # 生成 configure 文件
./configure
make
make check
sudo make install
sudo ldconfig
```

查看protoc版本

```shell
protoc --version
# libprotoc 3.17.3
```

查看链接库位置

```shell
pkg-config --cflags --libs protobuf
# -pthread  -lprotobuf -lpthread
```

若果你运行这段命令发生错误，你可以需要缺少相应的环境变量，你可以添加如下的环境变量

```shell
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/lib/pkgconfig"
```



## 2、Protocol Buffer语法

本小节翻译自[proto3 官方文档](https://developers.google.com/protocol-buffers/docs/proto3)

### 2.1 定义消息类型

​	现在来看一个简单的例子

```protobuf
// 下面这行用于指定protobuf的版本，若不指定，默认为proto2
// 指定版本这一行必须为第一行有效代码，即除开注释和空行的第一行
syntax = "proto3";

message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 result_per_page = 3;
}
```

​	`SearchRequest` 定义了3个字段(键值对)，分别对应你想要添加到`SearchRequest` 的数据。每个字段都包含一个name和类型

​	在上面的例子中，所有字段都是[scalar类型](https://developers.google.com/protocol-buffers/docs/proto3#scalar): 两个int类型(`page_number`和`result_per_page`)和一个string类型(`query`)。当然，你也可以使用符合类型，例如:[枚举](https://developers.google.com/protocol-buffers/docs/proto3#enum)和其他消息类型。

### 2.2 分配Field Numbers

​	正如你看到的那样，消息中的每个字段都有一个**唯一的编号 **。这些字段编号识别消息的二进制编码中的字段，所以在使用时不可以改变消息定义。需要注意的是1~15这些编号，会使用1个字节进行编码，这个字节包含字段编号和字段的类型(你可以在[Protocol Buffer Encoding](https://developers.google.com/protocol-buffers/docs/encoding#structure)了解其具体细节 )。16~2047会使用2个字节进行编码。所以你应该将1~15留给使用很频繁的字段。记得留下一些1~15的编号，以免以后有添加了会使用很频繁的字段。

​	你可以使用最小的字段编号是1，最大的字段编号是2^29=536,870,911。并且但是你不能够私有19000~19999的编号(`FieldDescriptor::kFirstReservedNumber`到`FieldDescriptor::kLastReservedNumber`)，因为Protocol Buffer的实现会使用这些编号。如果你在你的.`proto`文件中使用了这些编号，编译器不会正常运行。类似的你不能使用`预留`的字段编号。

### 2.3 指定字段规则

消息字段能够使用下面两条规则中的一条

+ singular:  符合该规则的字段，可以出现0次或者一次(但是不能出现两次及以上)。proto3中字段默认为singular。
+ `repeated`: 符合改规则的字段，可以重复任意次(包括0)，重复值的顺序会被保留。

proto3中，`repeated`修饰的字段默认会使用`packed`进行编码。通俗的来讲，repeated就是允许发送数组。

你可以在[Protocol Buffer Encoding](https://developers.google.com/protocol-buffers/docs/encoding#packed)了解pack的实现细节。

### 2.4 添加更多的消息

​	一个`.proto`文件可以定义多个消息。如果你定义了多个有关系的消息这会很有用。例如，如果你想定义一个与`SearchRequest `对应的响应消息，你可以将下面这样，将它们添加到一个`.proto`文件中。

```protobuf
message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 result_per_page = 3;
}

message SearchResponse {
 ...
}
```

### 2.5 注释

proto的注释风格与C/C++风格一致,使用`//`和`/* ...  */`。

```protobuf
/* SearchRequest represents a search query, with pagination options to
 * indicate which results to include in the response. */

message SearchRequest {
  string query = 1;
  int32 page_number = 2;  // Which page number do we want?
  int32 result_per_page = 3;  // Number of results to return per page.
}
```

### 2.6 保留字段

​	如果你在一次更新消息定义中，删除了一个字段，或者注释了它，以后的使用者可能会重新使用这个字段编号。如果这时候，他们同时在一些地方使用了旧版本的`.proto`文件，可能会导致数据损坏，隐私漏洞等严重的问题。如果你想确保你不会发生这样的问题，你可以使用`reserved`关键字来暂时保留你想删除的字段(编号，name)。如果以后有人使用了这些字段标识符，Protocol Buffer编译器警告他们别这样做。

```protobuf
message Foo {
  reserved 2, 15, 9 to 11;
  reserved "foo", "bar";
}
```

注意，你不能在同一个`reserved`块中混合使用字段名和字段编号。

### 2.7 编译.proto文件

​	你可以使用如下命令编译`.proto`文件

```sh
protoc --cpp_out=. xxxx.proto
# --cpp_out：文件的输出路径
# 具体的使用方式，可以通过protoc --help查看
```

​	当你在编译`.proto`文件时，编译器会根据参数会根据你选择的语言，来生成对应的目标代码，这些代码包括字段的getter和setter，序列化消息为二进制流，并将二进制流解析为对应的消息。

​	不同语言生成的目标文件如下：

​		**C++**：编译器会为每个`.proto`文件生成对应的`.h`和`.cc`文件，其中包含每个消息都一个与之对应的类。

​		**Java**： 编译器会为每个消息生成对应的`.java`文件，其中包含对应的类，也会生成一个对应的`Builder`类，来创建对应类的实例。

​	其他语言请看[这里](https://developers.google.com/protocol-buffers/docs/proto3#whats_generated_from_your_proto)。

​	你可以通过学习后面的示例程序，来学习C++中如何使用生成的文件。

### 2.8 默认值

​	当解析一个消息时，如果消息的二进制流，没有包含一些singular字段，这些字段会在解析的时候会被设置为默认值。不同类型的默认值如下：

+ **string**：默认为空字符串
+ **bytes**：默认为空bytes
+ **bool**：默认为false
+ **enum**：默认为枚举的类型的第一个元素，也就是0
+ **message**：具体的值取决于对应的语言

repeated字段的默认值为空(通常来说是一个空的`list`)

请看[这里](https://developers.google.com/protocol-buffers/docs/reference/overview)来了解默认值是如何工作的。

### 2.9 枚举

​	当你在定义一个消息时，你可能会想要消息的某些字段只能够是一个预先定义的值。比如说，你想在`SearchRequest`中添加一个`corpus`字段，该字段的取值只能是`UNIVERSAL`，`WEB`，`IMAGES`，`LOCAL`，NEWS，`PRODUCTS` or  `VIDEO`。在Protocol Buffer中你可以使用`enum`来为你的消息定义一些每一个可能用到的常量。

​	在下面的例子中我们添加了一个名为`Corpus`的`Enum`，和该类型的字段。

```protobuf
message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 result_per_page = 3;
  enum Corpus {
    UNIVERSAL = 0;
    WEB = 1;
    IMAGES = 2;
    LOCAL = 3;
    NEWS = 4;
    PRODUCTS = 5;
    VIDEO = 6;
  }
  Corpus corpus = 4;
}
```

​	正如你看到的那样，`Corpus` enum的第一个常量被映射到0：每个enum定义`必须`包含让一个被映射到0的常量作为它第一个元素。这是因为

+ enum定义中必须包含一个值为0的元素，这样我们就可以使用0来作为enum的默认值。
+ 需要将值为0的元素作为enum的第一个元素的，这主要是为了兼容`proto2`的语义：将第一个元素作为其默认值。

​	你可以将相同的值分配给不同的常量来定义别名，同时你需要将`allow_alias` option 设置为true，否则这个protocol编译器会输出一个错误信息。

```protobuf
message MyMessage1 {
  enum EnumAllowingAlias {
    option allow_alias = true;
    UNKNOWN = 0;
    STARTED = 1;
    RUNNING = 1;
  }
}
message MyMessage2 {
  enum EnumNotAllowingAlias {
    UNKNOWN = 0;
    STARTED = 1;
    // RUNNING = 1;  // Uncommenting this line will cause a compile error inside Google and a warning message outside.
  }
}
```

### 2.10 使用其他消息类型

​	你可以使用其他的消息类型来作为为字段的类型。例如，比如说你想要每个`SearchResponse`都包含一个`Result`消息，你可以定义一个`Result`消息类型在同一个`.proto`文件中，并且在`SearchResponse`的定义中添加一个对应类型的字段

```protobuf
message SearchResponse {
  repeated Result results = 1;
}

message Result {
  string url = 1;
  string title = 2;
  repeated string snippets = 3;
}
```

## 3、示例程序

github 地址: 

​	现在让我们用`Makefile`和`protobuf`来实现一个简单的程序。这个示例程序简单的使用多进程：一个进程作为client、一个进程作为server，还有管道用于这两个进程之间的通信，为了降低复杂性并不会使用锁来控制并发操作，其大致的运行流程如下图：

​	

​	![image-20210620224210229](img\image-20210620224210229.png)

​	该程序的功能只有一个就是通过userID查询用户信息，从而打印用户的游戏拥有情况和购买游戏花费的钱。

​	用户相关信息定义如下：

```protobuf
syntax = "proto3"; // 若不指定，默认为 proto2
package protobuf; // c++中会将其作为namespace
//
// 命名规则：
// message: 驼峰命名
// message字段: 下划线命名
// enum: 驼峰
// enum字段: 大写_下划线_大写
//
message Game {
	int32 game_id = 1;
	string name = 2;
	double price = 3;

	enum Type {
    ROLE_PLAY = 0;
    MOBA = 1;
    FPS = 2;
  }
	Type type = 5;
}

message User {
  string username = 1;
  string password = 2;
  int32 user_id = 3;
  uint32 age = 4;
  repeated Game own_games = 5;
}
```

 	现在让我们来看一下在C++如何操作`singular`字段

```c++
void BuildUser(User *user, int id, const char *name, int age) {
  user->set_age(age);
  user->set_user_id(id);
  user->set_username(name);
  user->set_password("123456"); // 程序中没有用到
}
```

​	你可以发现对于`singular`字段都是直接使用相应的set函数来为他设置值，在C++中`singular`字段都包含一个get和set函数，操作这些字段调用相关的函数几个。

​	然后再来看看如何操作repeated字段

```c++
void AddGame(User *user, Game *add_game) {
  Game *game = user->add_own_games();
  *game = *add_game;
}
```

​	正如你看到的那样，如果你想为用户字段添加一个游戏，你需要首先调用`add_user_games`，这会返回一个Game指针，你可以通过为该指针赋值来完成添加的全部工作。在C++中每个`repeated`都对应着一个类似于`vector`的数据结构。

​	让我们来看一看protoc生成的代码 ，看看编译器为你创建了那些类和函数，如果你查看`game_store.pb.h`，你可以看到`game_store.pb.h`中的每个消息都有一个对应的类。仔细看看`User`类，你可以看到编译器为每一个字段的都生成的访问方法，例如`username`，`user_id`，`own_games`，

```c++
// username
inline void clear_username(); // 清除username
inline const ::std::string& username() const;  // getter
inline void set_username(const ::std::string& value); // setter
inline void set_username(const char* value);  // setter
inline ::std::string* mutable_name(); //  返回允许修改的username字符串

// user_id
inline void clear_user_id();	// 将id设置0
inline int32_t user_id() const; // getter
inline void set_user_id(int32_t value); // setter

// own_games
inline int own_games_size() const;  // 获取own_games的长度
inline void clear_own_games();		
inline const RepeatedPtrField< ::protobuf::Game >& own_games() const // 获取own_games “vector"，
    																 // 可用于遍历所有game，不可修改
inline RepeatedPtrField< ::protobuf::Game >* mutable_own_games(); // 获取own_games “vector"，
    														      // 可以修改
inline const ::protobuf::Game& own_games(int index); // 不可变game
inline ::protobuf::Game* mutable_own_games(int index);  // 根据index获取一个可修改的game指针
inline ::tutorial::Person_PhoneNumber* add_own_games(); // 添加一个game，并返回一个指向game的														   指针，用于赋值。
// 序列化消息为string，这里的string并不是文本，而是二进制。
bool SerializeToString(string* output) const;
// 将字符串解析字符串为message
bool ParseFromString(const string& data); // 解析给定字符串到 message
bool SerializeToOstream(ostream* output) const;:// 将 message 写入给定的 C++ 的 ostream
bool ParseFromIstream(istream* input);: // 解析给定 C++ istream 到 message

```

​	到这里你应该知道了如何在C++使用Protocol Buffer，现在让我们来看看game_store中的其他部分。

​	首先会程序声明了User和Game静态数组，在初始化时，会根据下面的数据初始化这两个数组，注意user_id就是其对应下标加1。

```txt
游戏拥有情况：
  小A: 王者荣耀，原神
  小B: 穿越火线
  小C: 原神
  小D: 王者荣耀，穿越火线
```

​	然后在让我们看看如何使用多进程和管道来模拟一个server/client通信模型，如果你没有接触过`fork`和`pipe`系统调用的话，那么下面的代码可能会使你感到困惑。

```c++
  int child2parent[2]; // 用于parent向child发送数据
  int parent2child[2]; // 用于child向parent发送数据
  // 0为读，1为写
  pipe(child2parent);
  pipe(parent2child);

  int pid = fork();
  if (pid == 0) {           // child as server
    close(child2parent[1]); // 关闭child到parent的写端
    close(parent2child[0]); // 关闭parent到child的读端
    init();
    ShowInfo();
    cout << "server  started\n" << endl;
    while (read(child2parent[0], &user_id, sizeof(user_id)) != 0) {
      Lookup(user_id);
      string send_user_info = Lookup(user_id);
      write(parent2child[1], send_user_info.c_str(), send_user_info.size());
      // 这里会调用send_user_info的析构函数，不用主动释放
    }
    printf("server exit\n");
  } else {                  // parent as child
    close(child2parent[0]); // 关闭child到parent的读端
    close(parent2child[1]); // 关闭parent到child的写端
    int i = 1000000;  // 子进程初始化完成
    while (i--)
      ;
    while (true) {
      cout << "请输入你想查询的用户id(1~4,-1退出):" << endl;
      cin >> user_id; // 这里和上面的user_id不同
      if (user_id == -1) {
        close(child2parent[1]);
        close(parent2child[0]);
        printf("client exit\n");
        exit(0);
      }
      if (user_id<1 || user_id> 4) {
        continue;
      }
      write(child2parent[1], &user_id, sizeof(user_id));
      int n = read(parent2child[0], &buf, sizeof(buf));
      User user;
      user.ParseFromArray(buf, n);
      printf("###########################################\n");
      printf("%s 共花费%.2f元 喜欢玩:", UserPayMoney(&user),
             user.username().c_str());
      ShowUserGames(&user);
      printf("###########################################\n");
    }
```

​	在代码的最开始部分，会声明两个管道，一个用于子进程向父进程传输数据，一个用于父进程向子进程传输数据，虽然一个管道就可以实现双端通信，但是这样做会引起两个问题:

+ 管道的`read`只有在有数据到达或者不可能接收到数据的时候才会返回，不可能接收到数据指的是管道的所有写端都被关闭了，如果你不关闭自己的写端那么你很有可能被自己无限阻塞。
+ server端发送的数据可能会被自己接收到，因为server在`write`后，会立马`read`，client可能还没有读 ，就被server读取了。

​    随后会通过`fork`创建一个子进程作为server，父子进程执行的代码最开始都会关闭不需要使用的读端和写端。

​	server首先会初始化`User`和`Game`数组，然后就等待接收用户发送`user_id`，然后在将其对应用户信息序列化为string，并发送string对应的二进制数据(`str.c_str()`)给client。

​    client首先会等待server初始化完成，这里只是简单的通过`while`循环等待一个固定的时间，然后client就等待用户输入 `user_id`，用户输入之后，它会将其发送到server，并在`read`函数中等待server返回，接收到server返回的数据之后，它会将其解析为一个User Message，并打印部分该用户的信息。

