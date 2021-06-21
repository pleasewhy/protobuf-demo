#include "proto/game_store.pb.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>

using namespace protobuf;
using namespace std;

void BuildUser(User *user, int id, const char *name, int age) {
  user->set_age(age);
  user->set_user_id(id);
  user->set_username(name);
  user->set_password("123456");
}

Game *BuildGame(Game *game, int id, const char *name, Game_Type type,
                double price) {
  game->set_type(type);
  game->set_game_id(id);
  game->set_name(name);
  game->set_price(price);
  return game;
}

void AddGame(User *user, Game *add_game) {
  Game *game = user->add_own_games();
  *game = *add_game;
}

User users[4];
Game games[3];

/**
 * 小A: 王者荣耀，原神
 * 小B: 穿越火线
 * 小C: 原神
 * 小D: 王者荣耀，穿越火线
 */
void init() {
  Game *wzry =
      BuildGame(games, 1, "王者荣耀", Game_Type::Game_Type_MOBA, 19.99);
  Game *cf =
      BuildGame(games + 1, 2, "穿越火线", Game_Type::Game_Type_FPS, 25.55);
  Game *ys =
      BuildGame(games + 2, 3, "原神", Game_Type::Game_Type_ROLE_PLAY, 39.99);

  BuildUser(users, 1, "小A", 17);
  AddGame(users, wzry);
  AddGame(users, ys);

  BuildUser(users + 1, 2, "小B", 18);
  AddGame(users + 1, cf);

  BuildUser(users + 2, 3, "小C", 20);
  AddGame(users + 2, ys);

  BuildUser(users + 3, 4, "小D", 16);
  AddGame(users + 3, wzry);
  AddGame(users + 3, cf);
}

const char *GetTypeStr(Game_Type type) {
  if (Game_Type::Game_Type_FPS == type) {
    return "射击游戏";
  }
  if (Game_Type::Game_Type_MOBA == type) {
    return "MOBA";
  }
  if (Game_Type::Game_Type_ROLE_PLAY == type) {
    return "角色扮演";
  }
  return 0;
}

void ShowUserGames(User *user) {
  for (int j = 0; j < user->own_games_size(); j++) {
    printf("%s ", user->own_games().Get(j).name().c_str());
  }
  printf("\n");
}

double UserPayMoney(User *user) {
  double amount = 0;
  for (int j = 0; j < user->own_games_size(); j++) {
    amount += user->own_games().Get(j).price();
  }
  return amount;
}

void ShowInfo() {
  printf("游戏:\n");
  for (int i = 0; i < 3; i++) {
    printf("%s %.2f %s\n", games[i].name().c_str(), games[i].price(),
           GetTypeStr(games[i].type()));
  }
  printf("\n用户:\n");
  for (int i = 0; i < 4; i++) {
    User *user = users + i;
    printf("%s %d岁 played:", user->username().c_str(), user->age());
    ShowUserGames(user);
  }
  printf("\n");
}

string Lookup(int user_id) { return users[user_id - 1].SerializeAsString(); }

int main() {
  char buf[1024];
  int user_id;
  int child2parent[2]; // 用于parent向child发送数据
  int parent2child[2]; // 用于child向parent发送数据
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
    int i = 1000000;
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
  }
  return 0;
}